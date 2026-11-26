//===-- CodePageSupport.cpp - Code pages of resource scripts ----*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CodePageSupport.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/TextEncoding.h"
#include <algorithm>
#include <iterator>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

using namespace llvm;

namespace {
#include "BestFit1252.inc"

// 1252 as Windows reads it (0x81, 0x8D, 0x8F, 0x90, 0x9D stand for
// themselves).
const uint16_t CP1252High[32] = {
    0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
    0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};
} // namespace

bool rc::isDoubleByteCodePage(int CodePage) {
  switch (CodePage) {
  case 932:
  case 936:
  case 949:
  case 950:
  case 1361:
    return true;
  default:
    break;
  }
#ifdef _WIN32
  CPINFO Info;
  if (CodePage > 0 && CodePage != 65001 && ::GetCPInfo(CodePage, &Info))
    return Info.MaxCharSize == 2;
#endif
  return false;
}

bool rc::isCodePageLeadByte(int CodePage, unsigned char B) {
  switch (CodePage) {
  case 932:
    return (B >= 0x81 && B <= 0x9F) || (B >= 0xE0 && B <= 0xFC);
  case 936:
  case 949:
  case 950:
    return B >= 0x81 && B <= 0xFE;
  case 1361:
    return (B >= 0x84 && B <= 0xD3) || (B >= 0xD8 && B <= 0xDE) ||
           (B >= 0xE0 && B <= 0xF9);
  default:
    break;
  }
#ifdef _WIN32
  if (isDoubleByteCodePage(CodePage))
    return ::IsDBCSLeadByteEx(CodePage, B);
#endif
  return false;
}

bool rc::encodeBestFit(int CodePage, uint32_t C, SmallVectorImpl<char> &Out) {
  if (C < 0x80) {
    Out.push_back(C);
    return true;
  }
  if (CodePage == 65001) {
    char Buf[UNI_MAX_UTF8_BYTES_PER_CODE_POINT];
    char *P = Buf;
    if (!ConvertCodePointToUTF8(C, P))
      return false;
    Out.append(Buf, P);
    return true;
  }
  if (CodePage == 1252) {
    if (C >= 0xA0 && C <= 0xFF) {
      Out.push_back(C);
      return true;
    }
    for (unsigned I = 0; I != 32; ++I)
      if (CP1252High[I] == C) {
        Out.push_back(0x80 + I);
        return true;
      }
    auto *It = std::lower_bound(
        std::begin(BestFit1252), std::end(BestFit1252), C,
        [](const auto &E, uint32_t V) { return E.CP < V; });
    Out.push_back(It != std::end(BestFit1252) && It->CP == C ? It->Byte : '?');
    return true;
  }
#ifdef _WIN32
  wchar_t W[2];
  int WN = 1;
  if (C >= 0x10000) {
    W[0] = 0xD800 + ((C - 0x10000) >> 10);
    W[1] = 0xDC00 + ((C - 0x10000) & 0x3FF);
    WN = 2;
  } else {
    W[0] = C;
  }
  char Buf[8];
  int N = ::WideCharToMultiByte(CodePage, 0, W, WN, Buf, sizeof(Buf), nullptr,
                                nullptr);
  if (N <= 0)
    return false;
  Out.append(Buf, Buf + N);
  return true;
#else
  auto Conv = TextEncodingConverter::create(
      "UTF-8", ("." + Twine(CodePage)).str());
  if (!Conv)
    return false;
  char Buf[UNI_MAX_UTF8_BYTES_PER_CODE_POINT];
  char *P = Buf;
  if (!ConvertCodePointToUTF8(C, P))
    return false;
  SmallString<8> Res;
  std::error_code EC = Conv->convert(StringRef(Buf, P - Buf), Res);
  if (EC && EC != std::errc::result_out_of_range)
    return false;
  Out.append(Res.begin(), Res.end());
  return true;
#endif
}
