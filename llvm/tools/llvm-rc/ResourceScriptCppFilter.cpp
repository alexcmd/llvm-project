//===-- ResourceScriptCppFilter.cpp ----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//
//
// This file implements an interface defined in ResourceScriptCppFilter.h.
//
//===---------------------------------------------------------------------===//

#include "ResourceScriptCppFilter.h"
#include "llvm/ADT/StringExtras.h"

#include <vector>

using namespace llvm;

namespace {

class Filter {
public:
  explicit Filter(StringRef Input,
                  std::vector<std::pair<size_t, int>> *CodePages)
      : Data(Input), DataLength(Input.size()), CodePages(CodePages) {}

  std::string run();

private:
  // Parse the line, returning whether the line should be included in
  // the output.
  bool parseLine(StringRef Line);

  bool streamEof() const;

  StringRef Data;
  size_t DataLength;

  size_t Pos = 0;
  bool Outputting = true;
  std::vector<std::pair<size_t, int>> *CodePages;
  size_t OutputSize = 0;
};

std::string Filter::run() {
  std::vector<StringRef> Output;

  while (!streamEof() && Pos != StringRef::npos) {
    size_t LineStart = Pos;
    Pos = Data.find_first_of("\r\n", Pos);
    Pos = Data.find_first_not_of("\r\n", Pos);
    StringRef Line = Data.take_front(Pos).drop_front(LineStart);

    if (parseLine(Line)) {
      Output.push_back(Line);
      OutputSize += Line.size();
    }
  }

  return llvm::join(Output, "");
}

bool Filter::parseLine(StringRef Line) {
  Line = Line.ltrim();

  if (!Line.consume_front("#")) {
    // A normal content line, filtered according to the current mode.
    return Outputting;
  }

  // Found a preprocessing directive line. From here on, we always return
  // false since the preprocessing directives should be filtered out.

  // openrt RC-PRAGMA-CODEPAGE: rc.exe reads the text after
  // "#pragma code_page(N)" in code page N (observed: a UTF-8 copyright sign
  // after code_page(65001) is U+00A9); record where it takes effect.
  {
    StringRef P = Line.ltrim();
    if (Outputting && CodePages && P.consume_front("pragma")) {
      P = P.ltrim();
      if (P.consume_front("code_page")) {
        P = P.ltrim();
        if (P.consume_front("(")) {
          P = P.ltrim();
          int CP = 0;
          if (P.starts_with_insensitive("default"))
            CodePages->emplace_back(OutputSize, -1);
          else if (!P.consumeInteger(10, CP))
            CodePages->emplace_back(OutputSize, CP);
        }
      }
      return false;
    }
  }

  Line.consume_front("line");
  if (!Line.starts_with(" "))
    return false; // Not a line directive (pragma etc).

  // #line 123 "path/file.h"
  // # 123 "path/file.h" 1

  Line =
      Line.ltrim(); // There could be multiple spaces after the #line directive

  size_t N;
  if (Line.consumeInteger(10, N)) // Returns true to signify an error
    return false;

  Line = Line.ltrim();

  if (!Line.consume_front("\""))
    return false; // Malformed line, no quote found.

  // Split the string at the last quote (in case the path name had
  // escaped quotes as well).
  Line = Line.rsplit('"').first;

  StringRef Ext = Line.rsplit('.').second;

  if (Ext.equals_insensitive("h") || Ext.equals_insensitive("c")) {
    Outputting = false;
  } else {
    Outputting = true;
  }

  return false;
}

bool Filter::streamEof() const { return Pos == DataLength; }

} // anonymous namespace

namespace llvm {

std::string filterCppOutput(StringRef Input,
                            std::vector<std::pair<size_t, int>> *CodePages) {
  return Filter(Input, CodePages).run();
}

} // namespace llvm
