//===-- HeaderIncludeGen.cpp - Generate Header Includes -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/DiagnosticFrontend.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/DependencyOutputOptions.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

namespace {
class HeaderIncludesCallback : public PPCallbacks {
  SourceManager &SM;
  raw_ostream *OutputFile;
  const DependencyOutputOptions &DepOpts;
  unsigned CurrentIncludeDepth;
  bool HasProcessedPredefines;
  bool OwnsOutputFile;
  bool ShowAllHeaders;
  bool ShowDepth;
  bool MSStyle;
  // cl.exe spelling of the include being entered (set by InclusionDirective).
  std::string PendingIncludePath;

public:
  HeaderIncludesCallback(const Preprocessor *PP, bool ShowAllHeaders_,
                         raw_ostream *OutputFile_,
                         const DependencyOutputOptions &DepOpts,
                         bool OwnsOutputFile_, bool ShowDepth_, bool MSStyle_)
      : SM(PP->getSourceManager()), OutputFile(OutputFile_), DepOpts(DepOpts),
        CurrentIncludeDepth(0), HasProcessedPredefines(false),
        OwnsOutputFile(OwnsOutputFile_), ShowAllHeaders(ShowAllHeaders_),
        ShowDepth(ShowDepth_), MSStyle(MSStyle_) {}

  ~HeaderIncludesCallback() override {
    if (OwnsOutputFile)
      delete OutputFile;
  }

  HeaderIncludesCallback(const HeaderIncludesCallback &) = delete;
  HeaderIncludesCallback &operator=(const HeaderIncludesCallback &) = delete;

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override;

  void FileSkipped(const FileEntryRef &SkippedFile, const Token &FilenameTok,
                   SrcMgr::CharacteristicKind FileType) override;

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *SuggestedModule,
                          bool ModuleImported,
                          SrcMgr::CharacteristicKind FileType) override;

private:
  bool ShouldShowHeader(SrcMgr::CharacteristicKind HeaderType) {
    if (!DepOpts.IncludeSystemHeaders && isSystem(HeaderType))
      return false;

    // Show the current header if we are (a) past the predefines, or (b) showing
    // all headers and in the predefines at a depth past the initial file and
    // command line buffers.
    return (HasProcessedPredefines ||
            (ShowAllHeaders && CurrentIncludeDepth > 2));
  }
};

/// A callback for emitting header usage information to a file in JSON. Each
/// line in the file is a JSON object that includes the source file name and
/// the list of headers directly or indirectly included from it. For example:
///
/// {"source":"/tmp/foo.c",
///  "includes":["/usr/include/stdio.h", "/usr/include/stdlib.h"]}
///
/// To reduce the amount of data written to the file, we only record system
/// headers that are directly included from a file that isn't in the system
/// directory.
class HeaderIncludesJSONCallback : public PPCallbacks {
  SourceManager &SM;
  raw_ostream *OutputFile;
  bool OwnsOutputFile;
  SmallVector<std::string, 16> IncludedHeaders;

public:
  HeaderIncludesJSONCallback(const Preprocessor *PP, raw_ostream *OutputFile_,
                             bool OwnsOutputFile_)
      : SM(PP->getSourceManager()), OutputFile(OutputFile_),
        OwnsOutputFile(OwnsOutputFile_) {}

  ~HeaderIncludesJSONCallback() override {
    if (OwnsOutputFile)
      delete OutputFile;
  }

  HeaderIncludesJSONCallback(const HeaderIncludesJSONCallback &) = delete;
  HeaderIncludesJSONCallback &
  operator=(const HeaderIncludesJSONCallback &) = delete;

  void EndOfMainFile() override;

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override;

  void FileSkipped(const FileEntryRef &SkippedFile, const Token &FilenameTok,
                   SrcMgr::CharacteristicKind FileType) override;
};

/// A callback for emitting direct header and module usage information to a
/// file in JSON. The output format is like HeaderIncludesJSONCallback but has
/// an array of separate entries, one for each non-system source file used in
/// the compilation showing only the direct includes and imports from that file.
class HeaderIncludesDirectPerFileCallback : public PPCallbacks {
  struct HeaderIncludeInfo {
    SourceLocation Location;
    FileEntryRef File;
    const Module *ImportedModule;

    HeaderIncludeInfo(SourceLocation Location, FileEntryRef File,
                      const Module *ImportedModule)
        : Location(Location), File(File), ImportedModule(ImportedModule) {}
  };

  SourceManager &SM;
  HeaderSearch &HSI;
  raw_ostream *OutputFile;
  bool OwnsOutputFile;
  using DependencyMap =
      llvm::DenseMap<FileEntryRef, SmallVector<HeaderIncludeInfo>>;
  DependencyMap Dependencies;

public:
  HeaderIncludesDirectPerFileCallback(const Preprocessor *PP,
                                      raw_ostream *OutputFile_,
                                      bool OwnsOutputFile_)
      : SM(PP->getSourceManager()), HSI(PP->getHeaderSearchInfo()),
        OutputFile(OutputFile_), OwnsOutputFile(OwnsOutputFile_) {}

  ~HeaderIncludesDirectPerFileCallback() override {
    if (OwnsOutputFile)
      delete OutputFile;
  }

  HeaderIncludesDirectPerFileCallback(
      const HeaderIncludesDirectPerFileCallback &) = delete;
  HeaderIncludesDirectPerFileCallback &
  operator=(const HeaderIncludesDirectPerFileCallback &) = delete;

  void EndOfMainFile() override;

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *SuggestedModule,
                          bool ModuleImported,
                          SrcMgr::CharacteristicKind FileType) override;

  void moduleImport(SourceLocation ImportLoc, ModuleIdPath Path,
                    const Module *Imported) override;
};
}

// cl.exe spelling of an include, as /showIncludes and /sourceDependencies print
// it. A quoted include found next to the including file is that directory made
// absolute, a backslash, then the spelled name. Any other include is its search
// directory as given, a backslash, then the name.
static std::string clIncludePath(const SourceManager &SM,
                                 SourceLocation HashLoc, StringRef SearchPath,
                                 StringRef RelativePath, bool IsAngled) {
  auto AbsDir = [](StringRef D) {
    SmallString<256> P(D.empty() ? StringRef(".") : D);
    sys::fs::make_absolute(P);
    sys::path::remove_dots(P, /*remove_dot_dot=*/true);
    return P;
  };
  SmallString<256> IncDir =
      AbsDir(sys::path::parent_path(SM.getFilename(HashLoc)));
  if (!IsAngled && StringRef(IncDir).equals_insensitive(AbsDir(SearchPath))) {
    SmallString<256> Native;
    sys::path::native(IncDir, Native, sys::path::Style::windows_backslash);
    return (Native + "\\" + RelativePath).str().str();
  }
  if (SearchPath.empty())
    return RelativePath.str();
  return (SearchPath + "\\" + RelativePath).str();
}

// /sourceDependencies spelling: full path, backslashes, lower case.
static std::string clDependencyPath(StringRef ClPath) {
  SmallString<256> P(ClPath);
  sys::fs::make_absolute(P);
  sys::path::remove_dots(P, /*remove_dot_dot=*/true,
                         sys::path::Style::windows_backslash);
  for (char &C : P)
    C = llvm::toLower(C == '/' ? '\\' : C);
  return P.str().str();
}

static void PrintHeaderInfo(raw_ostream *OutputFile, StringRef Filename,
                            bool ShowDepth, unsigned CurrentIncludeDepth,
                            bool MSStyle) {
  // Write to a temporary string to avoid unnecessary flushing on errs().
  SmallString<512> Pathname(Filename);
  if (!MSStyle)
    Lexer::Stringify(Pathname);

  SmallString<256> Msg;
  if (MSStyle)
    Msg += "Note: including file:";

  if (ShowDepth) {
    // The main source file is at depth 1, so skip one dot.
    for (unsigned i = 1; i != CurrentIncludeDepth; ++i)
      Msg += MSStyle ? ' ' : '.';

    if (!MSStyle)
      Msg += ' ';
  }
  Msg += Pathname;
  Msg += '\n';

  *OutputFile << Msg;
  OutputFile->flush();
}

void clang::AttachHeaderIncludeGen(Preprocessor &PP,
                                   const DependencyOutputOptions &DepOpts,
                                   bool ShowAllHeaders, StringRef OutputPath,
                                   bool ShowDepth, bool MSStyle) {
  raw_ostream *OutputFile = &llvm::errs();
  bool OwnsOutputFile = false;

  // Choose output stream, when printing in cl.exe /showIncludes style.
  if (MSStyle) {
    switch (DepOpts.ShowIncludesDest) {
    default:
      llvm_unreachable("Invalid destination for /showIncludes output!");
    case ShowIncludesDestination::Stderr:
      OutputFile = &llvm::errs();
      break;
    case ShowIncludesDestination::Stdout:
      OutputFile = &llvm::outs();
      break;
    }
  }

  // Open the output file, if used.
  if (!OutputPath.empty()) {
    std::error_code EC;
    llvm::raw_fd_ostream *OS = new llvm::raw_fd_ostream(
        OutputPath.str(), EC,
        llvm::sys::fs::OF_Append | llvm::sys::fs::OF_TextWithCRLF);
    if (EC) {
      PP.getDiagnostics().Report(clang::diag::warn_fe_cc_print_header_failure)
          << EC.message();
      delete OS;
    } else {
      OS->SetUnbuffered();
      OutputFile = OS;
      OwnsOutputFile = true;
    }
  }

  switch (DepOpts.HeaderIncludeFormat) {
  case HIFMT_None:
    llvm_unreachable("unexpected header format kind");
  case HIFMT_Textual: {
    assert(DepOpts.HeaderIncludeFiltering == HIFIL_None &&
           "header filtering is currently always disabled when output format is"
           "textual");
    // Print header info for extra headers, pretending they were discovered by
    // the regular preprocessor. The primary use case is to support proper
    // generation of Make / Ninja file dependencies for implicit includes, such
    // as sanitizer ignorelists. It's only important for cl.exe compatibility,
    // the GNU way to generate rules is -M / -MM / -MD / -MMD.
    for (const auto &Header : DepOpts.ExtraDeps)
      PrintHeaderInfo(OutputFile, Header.first, ShowDepth, 2, MSStyle);
    PP.addPPCallbacks(std::make_unique<HeaderIncludesCallback>(
        &PP, ShowAllHeaders, OutputFile, DepOpts, OwnsOutputFile, ShowDepth,
        MSStyle));
    break;
  }
  case HIFMT_JSON:
    switch (DepOpts.HeaderIncludeFiltering) {
    default:
      llvm_unreachable("Unknown HeaderIncludeFilteringKind enum");
    case HIFIL_Only_Direct_System:
      PP.addPPCallbacks(std::make_unique<HeaderIncludesJSONCallback>(
          &PP, OutputFile, OwnsOutputFile));
      break;
    case HIFIL_Direct_Per_File:
      PP.addPPCallbacks(std::make_unique<HeaderIncludesDirectPerFileCallback>(
          &PP, OutputFile, OwnsOutputFile));
      break;
    }
    break;
  }
}

void HeaderIncludesCallback::FileChanged(SourceLocation Loc,
                                         FileChangeReason Reason,
                                         SrcMgr::CharacteristicKind NewFileType,
                                         FileID PrevFID) {
  // Unless we are exiting a #include, make sure to skip ahead to the line the
  // #include directive was at.
  PresumedLoc UserLoc = SM.getPresumedLoc(Loc);
  if (UserLoc.isInvalid())
    return;

  // Adjust the current include depth.
  if (Reason == PPCallbacks::EnterFile) {
    ++CurrentIncludeDepth;
  } else if (Reason == PPCallbacks::ExitFile) {
    if (CurrentIncludeDepth)
      --CurrentIncludeDepth;

    // We track when we are done with the predefines by watching for the first
    // place where we drop back to a nesting depth of 1.
    if (CurrentIncludeDepth == 1 && !HasProcessedPredefines)
      HasProcessedPredefines = true;

    return;
  } else {
    return;
  }

  if (!ShouldShowHeader(NewFileType))
    return;

  unsigned IncludeDepth = CurrentIncludeDepth;
  if (!HasProcessedPredefines)
    --IncludeDepth; // Ignore indent from <built-in>.

  // FIXME: Identify headers in a more robust way than comparing their name to
  // "<command line>" and "<built-in>" in a bunch of places.
  if (Reason == PPCallbacks::EnterFile &&
      UserLoc.getFilename() != StringRef("<command line>")) {
    PrintHeaderInfo(OutputFile,
                    PendingIncludePath.empty()
                        ? UserLoc.getFilename()
                        : StringRef(PendingIncludePath),
                    ShowDepth, IncludeDepth, MSStyle);
  }
  PendingIncludePath.clear();
}

void HeaderIncludesCallback::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, OptionalFileEntryRef File,
    StringRef SearchPath, StringRef RelativePath,
    const Module *SuggestedModule, bool ModuleImported,
    SrcMgr::CharacteristicKind FileType) {
  PendingIncludePath.clear();
  if (MSStyle && File)
    PendingIncludePath =
        clIncludePath(SM, HashLoc, SearchPath, RelativePath, IsAngled);
}

void HeaderIncludesCallback::FileSkipped(const FileEntryRef &SkippedFile,
                                         const Token &FilenameTok,
                                         SrcMgr::CharacteristicKind FileType) {
  if (!DepOpts.ShowSkippedHeaderIncludes)
    return;

  if (!ShouldShowHeader(FileType))
    return;

  PrintHeaderInfo(OutputFile, SkippedFile.getName(), ShowDepth,
                  CurrentIncludeDepth + 1, MSStyle);
}

void HeaderIncludesJSONCallback::EndOfMainFile() {
  OptionalFileEntryRef FE = SM.getFileEntryRefForID(SM.getMainFileID());
  SmallString<256> MainFile;
  if (FE) {
    MainFile += FE->getName();
    SM.getFileManager().makeAbsolutePath(MainFile);
  }

  std::string Str;
  llvm::raw_string_ostream OS(Str);
  llvm::json::OStream JOS(OS);
  JOS.object([&] {
    JOS.attribute("source", MainFile.c_str());
    JOS.attributeArray("includes", [&] {
      llvm::StringSet<> SeenHeaders;
      for (const std::string &H : IncludedHeaders)
        if (SeenHeaders.insert(H).second)
          JOS.value(H);
    });
  });
  OS << "\n";

  if (OutputFile->get_kind() == raw_ostream::OStreamKind::OK_FDStream) {
    llvm::raw_fd_ostream *FDS = static_cast<llvm::raw_fd_ostream *>(OutputFile);
    if (auto L = FDS->lock())
      *OutputFile << Str;
  } else
    *OutputFile << Str;
}

/// Determine whether the header file should be recorded. The header file should
/// be recorded only if the header file is a system header and the current file
/// isn't a system header.
static bool shouldRecordNewFile(SrcMgr::CharacteristicKind NewFileType,
                                SourceLocation PrevLoc, SourceManager &SM) {
  return SrcMgr::isSystem(NewFileType) && !SM.isInSystemHeader(PrevLoc);
}

void HeaderIncludesJSONCallback::FileChanged(
    SourceLocation Loc, FileChangeReason Reason,
    SrcMgr::CharacteristicKind NewFileType, FileID PrevFID) {
  if (PrevFID.isInvalid() ||
      !shouldRecordNewFile(NewFileType, SM.getLocForStartOfFile(PrevFID), SM))
    return;

  // Unless we are exiting a #include, make sure to skip ahead to the line the
  // #include directive was at.
  PresumedLoc UserLoc = SM.getPresumedLoc(Loc);
  if (UserLoc.isInvalid())
    return;

  if (Reason == PPCallbacks::EnterFile &&
      UserLoc.getFilename() != StringRef("<command line>"))
    IncludedHeaders.push_back(UserLoc.getFilename());
}

void HeaderIncludesJSONCallback::FileSkipped(
    const FileEntryRef &SkippedFile, const Token &FilenameTok,
    SrcMgr::CharacteristicKind FileType) {
  if (!shouldRecordNewFile(FileType, FilenameTok.getLocation(), SM))
    return;

  IncludedHeaders.push_back(SkippedFile.getName().str());
}

void HeaderIncludesDirectPerFileCallback::EndOfMainFile() {
  if (Dependencies.empty())
    return;

  // Sort the files so that the output does not depend on the DenseMap order.
  SmallVector<FileEntryRef> SourceFiles;
  for (auto F = Dependencies.begin(), FEnd = Dependencies.end(); F != FEnd;
       ++F) {
    SourceFiles.push_back(F->first);
  }
  llvm::sort(SourceFiles, [](const FileEntryRef &LHS, const FileEntryRef &RHS) {
    return LHS.getUID() < RHS.getUID();
  });

  std::string Str;
  llvm::raw_string_ostream OS(Str);
  llvm::json::OStream JOS(OS);
  JOS.object([&] {
    JOS.attribute("version", "2.0.0");
    JOS.attributeArray("dependencies", [&] {
      for (const auto &S : SourceFiles) {
        JOS.object([&] {
          SmallVector<HeaderIncludeInfo> &Deps = Dependencies[S];
          JOS.attribute("source", S.getName().str());
          JOS.attributeArray("includes", [&] {
            for (unsigned I = 0, N = Deps.size(); I != N; ++I) {
              if (!Deps[I].ImportedModule) {
                JOS.object([&] {
                  JOS.attribute("location", Deps[I].Location.printToString(SM));
                  JOS.attribute("file", Deps[I].File.getName());
                });
              }
            }
          });
          JOS.attributeArray("imports", [&] {
            for (unsigned I = 0, N = Deps.size(); I != N; ++I) {
              if (Deps[I].ImportedModule) {
                JOS.object([&] {
                  JOS.attribute("location", Deps[I].Location.printToString(SM));
                  JOS.attribute(
                      "module",
                      Deps[I].ImportedModule->getTopLevelModuleName());
                  JOS.attribute("file", Deps[I].File.getName());
                });
              }
            }
          });
        });
      }
    });
  });

  OS << "\n";

  if (OutputFile->get_kind() == raw_ostream::OStreamKind::OK_FDStream) {
    llvm::raw_fd_ostream *FDS = static_cast<llvm::raw_fd_ostream *>(OutputFile);
    if (auto L = FDS->lock())
      *OutputFile << Str;
  } else
    *OutputFile << Str;
}

void HeaderIncludesDirectPerFileCallback::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, OptionalFileEntryRef File,
    StringRef SearchPath, StringRef RelativePath, const Module *SuggestedModule,
    bool ModuleImported, SrcMgr::CharacteristicKind FileType) {
  if (!File)
    return;

  SourceLocation Loc = SM.getExpansionLoc(HashLoc);
  if (SM.isInSystemHeader(Loc))
    return;
  OptionalFileEntryRef FromFile = SM.getFileEntryRefForID(SM.getFileID(Loc));
  if (!FromFile)
    return;

  FileEntryRef HeaderOrModuleMapFile = *File;
  if (ModuleImported && SuggestedModule) {
    OptionalFileEntryRef ModuleMapFile =
        HSI.getModuleMap().getModuleMapFileForUniquing(SuggestedModule);
    if (ModuleMapFile) {
      HeaderOrModuleMapFile = *ModuleMapFile;
    }
  }

  HeaderIncludeInfo DependenciesEntry(
      Loc, HeaderOrModuleMapFile, (ModuleImported ? SuggestedModule : nullptr));
  Dependencies[*FromFile].push_back(DependenciesEntry);
}

void HeaderIncludesDirectPerFileCallback::moduleImport(SourceLocation ImportLoc,
                                                       ModuleIdPath Path,
                                                       const Module *Imported) {
  if (!Imported)
    return;

  SourceLocation Loc = SM.getExpansionLoc(ImportLoc);
  if (SM.isInSystemHeader(Loc))
    return;
  OptionalFileEntryRef FromFile = SM.getFileEntryRefForID(SM.getFileID(Loc));
  if (!FromFile)
    return;

  OptionalFileEntryRef ModuleMapFile =
      HSI.getModuleMap().getModuleMapFileForUniquing(Imported);
  if (!ModuleMapFile)
    return;

  HeaderIncludeInfo DependenciesEntry(Loc, *ModuleMapFile, Imported);
  Dependencies[*FromFile].push_back(DependenciesEntry);
}

namespace {
/// Collects the entered include files and writes the cl.exe /sourceDependencies
/// JSON (Version 1.2, CRLF, 4-space indent) at the end of the main file.
class SourceDependenciesCallback : public PPCallbacks {
  SourceManager &SM;
  std::string OutPath;
  std::string PendingPath;
  std::vector<std::string> Includes;

public:
  SourceDependenciesCallback(SourceManager &SM, StringRef OutPath)
      : SM(SM), OutPath(OutPath) {}

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *SuggestedModule,
                          bool ModuleImported,
                          SrcMgr::CharacteristicKind FileType) override {
    PendingPath.clear();
    if (File)
      PendingPath =
          clIncludePath(SM, HashLoc, SearchPath, RelativePath, IsAngled);
  }

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override {
    if (Reason != PPCallbacks::EnterFile || PendingPath.empty())
      return;
    Includes.push_back(clDependencyPath(PendingPath));
    PendingPath.clear();
  }

  void EndOfMainFile() override;
};

void SourceDependenciesCallback::EndOfMainFile() {
  OptionalFileEntryRef FE = SM.getFileEntryRefForID(SM.getMainFileID());
  if (!FE)
    return;
  StringRef MainName = FE->getName();

  std::unique_ptr<raw_ostream> Owned;
  raw_ostream *OS = &llvm::outs();
  if (OutPath != "-") {
    SmallString<256> Path(OutPath);
    bool EndsWithSep = StringRef(OutPath).ends_with("\\") ||
                       StringRef(OutPath).ends_with("/");
    if (EndsWithSep || sys::fs::is_directory(OutPath)) {
      if (!EndsWithSep)
        Path += '\\';
      Path += sys::path::filename(MainName);
      Path += ".json";
    }
    std::error_code EC;
    auto FOS = std::make_unique<llvm::raw_fd_ostream>(
        Path, EC, llvm::sys::fs::OF_TextWithCRLF);
    if (EC)
      return;
    Owned = std::move(FOS);
    OS = Owned.get();
  }

  std::string Source = clDependencyPath(MainName);
  llvm::json::OStream J(*OS, 4);
  J.object([&] {
    J.attribute("Version", "1.2");
    J.attributeObject("Data", [&] {
      J.attribute("Source", Source);
      J.attribute("ProvidedModule", "");
      J.attributeArray("Includes", [&] {
        for (const std::string &I : Includes)
          J.value(llvm::json::Value(I));
      });
    });
  });
  OS->flush();
}
} // namespace

void clang::AttachSourceDependencies(Preprocessor &PP,
                                     const DependencyOutputOptions &DepOpts) {
  PP.addPPCallbacks(std::make_unique<SourceDependenciesCallback>(
      PP.getSourceManager(), DepOpts.SourceDependenciesFile));
}
