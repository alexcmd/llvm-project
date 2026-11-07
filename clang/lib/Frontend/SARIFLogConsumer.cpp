//===--- SARIFLogConsumer.cpp - SARIF log file of the diagnostics ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/SARIFLogConsumer.h"
#include "clang/Basic/CLWarnings.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/IOSandbox.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <optional>
#include <string>
#include <vector>

using namespace clang;

namespace {

struct Loc {
  std::string URI;
  unsigned Line = 0, Column = 0;
};

struct Related {
  std::optional<Loc> Where;
  std::string Text;
};

struct Result {
  std::string RuleId;
  bool IsError = false;
  std::string Text;
  std::optional<Loc> Where;
  std::vector<Related> Notes;
};

class SARIFLogConsumer : public DiagnosticConsumer {
  std::string Path;
  std::vector<Result> Results;
  bool Written = false;
  bool LastKept = false;
  llvm::DenseMap<unsigned, unsigned> RuleCache;

  static std::string toURI(StringRef File) {
    SmallString<256> P(File);
    // The log is a side output like /showIncludes: outside the cc1 sandbox.
    auto BypassSandbox = llvm::sys::sandbox::scopedDisable();
    llvm::sys::fs::make_absolute(P);
    llvm::sys::path::remove_dots(P, /*remove_dot_dot=*/true);
    llvm::sys::path::native(P, llvm::sys::path::Style::windows_slash);
    std::string Out = "file:///";
    for (char C : P) {
      if (C == '\\')
        Out += '/';
      else if (C == ' ' || C == '%' || C == '#' || (unsigned char)C >= 0x80) {
        char Buf[4];
        snprintf(Buf, sizeof(Buf), "%%%02X", (unsigned char)C);
        Out += Buf;
      } else
        Out += C;
    }
    if (llvm::sys::path::is_absolute(File) && File.starts_with("/"))
      Out = "file://" + Out.substr(8);
    return Out;
  }

  static std::optional<Loc> location(const Diagnostic &Info) {
    if (!Info.getLocation().isValid() || !Info.hasSourceManager())
      return std::nullopt;
    const SourceManager &SM = Info.getSourceManager();
    PresumedLoc PL = SM.getPresumedLoc(SM.getFileLoc(Info.getLocation()));
    if (PL.isInvalid())
      return std::nullopt;
    return Loc{toURI(PL.getFilename()), PL.getLine(), PL.getColumn()};
  }

  std::string ruleId(const Diagnostic &Info) {
    unsigned DiagID = Info.getID();
    StringRef Opt =
        Info.getDiags()->getDiagnosticIDs()->getWarningOptionForDiag(DiagID);
    if (Opt.empty())
      return {};
    auto It = RuleCache.find(DiagID);
    unsigned CL = 0;
    if (It != RuleCache.end()) {
      CL = It->second;
    } else {
      if (auto G = DiagnosticIDs::getGroupForWarningOption(Opt))
        for (unsigned ID = 4000; ID != 6000 && !CL; ++ID)
          if (diagGroupFromCLWarningID(ID) == G)
            CL = ID;
      RuleCache[DiagID] = CL;
    }
    // CLANG-CL-GAP(sarif-ruleid): ids without a cl number keep the clang
    // option name.
    return CL ? "C" + std::to_string(CL) : ("-W" + Opt).str();
  }

  static void writeLoc(llvm::json::OStream &J, const Loc &L) {
    J.attributeObject("physicalLocation", [&] {
      J.attributeObject("artifactLocation",
                        [&] { J.attribute("uri", L.URI); });
      J.attributeObject("region", [&] {
        J.attribute("startLine", L.Line);
        J.attribute("startColumn", L.Column);
      });
    });
  }

  void write() {
    if (Written)
      return;
    Written = true;
    auto BypassSandbox = llvm::sys::sandbox::scopedDisable();
    std::error_code EC;
    llvm::raw_fd_ostream OS(Path, EC, llvm::sys::fs::OF_TextWithCRLF);
    if (EC)
      return;
    llvm::json::OStream J(OS, 4);
    J.object([&] {
      J.attribute("version", "2.1.0");
      J.attribute("$schema", "https://schemastore.azurewebsites.net/schemas/"
                             "json/sarif-2.1.0-rtm.5.json");
      J.attributeArray("runs", [&] {
        J.object([&] {
          J.attributeArray("results", [&] {
            for (const Result &R : Results)
              J.object([&] {
                if (!R.RuleId.empty())
                  J.attribute("ruleId", R.RuleId);
                if (R.IsError)
                  J.attribute("level", "error");
                J.attributeObject("message",
                                  [&] { J.attribute("text", R.Text); });
                if (R.Where)
                  J.attributeArray("locations",
                                   [&] { J.object([&] { writeLoc(J, *R.Where); }); });
                if (!R.Notes.empty())
                  J.attributeArray("relatedLocations", [&] {
                    for (size_t I = 0; I != R.Notes.size(); ++I)
                      J.object([&] {
                        J.attribute("id", int64_t(I));
                        if (R.Notes[I].Where)
                          writeLoc(J, *R.Notes[I].Where);
                        J.attributeObject("message", [&] {
                          J.attribute("text", R.Notes[I].Text);
                        });
                      });
                  });
              });
          });
          J.attributeObject("tool", [&] {
            J.attributeObject("driver", [&] {
              J.attribute("name", "openrt");
              J.attributeObject("shortDescription", [&] {
                J.attribute("text", "openrt C/C++ compiler warnings and errors");
              });
            });
          });
        });
      });
    });
    OS << "\n";
  }

public:
  explicit SARIFLogConsumer(StringRef Path) : Path(Path.str()) {}
  ~SARIFLogConsumer() override { write(); }

  void HandleDiagnostic(DiagnosticsEngine::Level Level,
                        const Diagnostic &Info) override {
    DiagnosticConsumer::HandleDiagnostic(Level, Info);
    SmallString<256> Text;
    if (Level == DiagnosticsEngine::Note) {
      if (LastKept && !Results.empty()) {
        Info.FormatDiagnostic(Text);
        Results.back().Notes.push_back({location(Info), std::string(Text)});
      }
      return;
    }
    LastKept = Level >= DiagnosticsEngine::Warning;
    if (!LastKept)
      return;
    Info.FormatDiagnostic(Text);
    Result R;
    R.IsError = Level >= DiagnosticsEngine::Error;
    R.RuleId = ruleId(Info);
    R.Text = std::string(Text);
    R.Where = location(Info);
    Results.push_back(std::move(R));
  }

  void EndSourceFile() override {
    Written = false;
    write();
  }
};

} // namespace

std::unique_ptr<DiagnosticConsumer>
clang::createSARIFLogConsumer(StringRef Path) {
  return std::make_unique<SARIFLogConsumer>(Path);
}
