//===-- MSVCPaths.h - MSVC path-parsing helpers -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_WINDOWSDRIVER_MSVCPATHS_H
#define LLVM_WINDOWSDRIVER_MSVCPATHS_H

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"
#include <optional>
#include <string>

namespace llvm {

namespace vfs {
class FileSystem;
}

enum class SubDirectoryType {
  Bin,
  Include,
  Lib,
};

enum class ToolsetLayout {
  OlderVS,
  VS2017OrNewer,
  DevDivInternal,
};

// Windows SDKs and VC Toolchains group their contents into subdirectories based
// on the target architecture. This function converts an llvm::Triple::ArchType
// to the corresponding subdirectory name.
LLVM_ABI const char *archToWindowsSDKArch(llvm::Triple::ArchType Arch);

// Similar to the above function, but for Visual Studios before VS2017.
LLVM_ABI const char *archToLegacyVCArch(llvm::Triple::ArchType Arch);

// Similar to the above function, but for DevDiv internal builds.
LLVM_ABI const char *archToDevDivInternalArch(llvm::Triple::ArchType Arch);

LLVM_ABI bool appendArchToWindowsSDKLibPath(int SDKMajor,
                                            llvm::SmallString<128> LibPath,
                                            llvm::Triple::ArchType Arch,
                                            std::string &path);

// Get the path to a specific subdirectory in the current toolchain for
// a given target architecture.
// VS2017 changed the VC toolchain layout, so this should be used instead
// of hardcoding paths.
LLVM_ABI std::string getSubDirectoryPath(SubDirectoryType Type,
                                         ToolsetLayout VSLayout,
                                         const std::string &VCToolChainPath,
                                         llvm::Triple::ArchType TargetArch,
                                         llvm::StringRef SubdirParent = "");

// Check if the Include path of a specified version of Visual Studio contains
// specific header files. If not, they are probably shipped with Universal CRT.
LLVM_ABI bool useUniversalCRT(ToolsetLayout VSLayout,
                              const std::string &VCToolChainPath,
                              llvm::Triple::ArchType TargetArch,
                              llvm::vfs::FileSystem &VFS);

/// Get Windows SDK installation directory.
LLVM_ABI bool getWindowsSDKDir(vfs::FileSystem &VFS,
                               std::optional<llvm::StringRef> WinSdkDir,
                               std::optional<llvm::StringRef> WinSdkVersion,
                               std::optional<llvm::StringRef> WinSysRoot,
                               std::string &Path, int &Major,
                               std::string &WindowsSDKIncludeVersion,
                               std::string &WindowsSDKLibVersion);

LLVM_ABI bool
getUniversalCRTSdkDir(vfs::FileSystem &VFS,
                      std::optional<llvm::StringRef> WinSdkDir,
                      std::optional<llvm::StringRef> WinSdkVersion,
                      std::optional<llvm::StringRef> WinSysRoot,
                      std::string &Path, std::string &UCRTVersion);

// Check command line arguments to try and find a toolchain.
LLVM_ABI bool
findVCToolChainViaCommandLine(vfs::FileSystem &VFS,
                              std::optional<llvm::StringRef> VCToolsDir,
                              std::optional<llvm::StringRef> VCToolsVersion,
                              std::optional<llvm::StringRef> WinSysRoot,
                              std::string &Path, ToolsetLayout &VSLayout);

// BIN-RELATIVE-LOOKUP: openrt-<ver> layout around the tool's own bin dir:
//   <root>\bin\clang-cl.exe, lld-link.exe   (BinDir)
//   <root>\include, <root>\lib\<arch>       (x64/x86/arm64/arm, as VS2017+)
//   <root>\manifest.json                    marker; must be a regular file
// On a hit Path = <root>, VSLayout = VS2017OrNewer (as /vctoolsdir:<root>).
// manifest.json "sdk": {"root": ..., "version": ...} -> SDKRoot/SDKVersion
// (empty if absent; relative root is taken against <root>). Malformed json
// keeps the layout and yields no SDK info. Callers decide precedence.
LLVM_ABI bool findVCToolChainViaBinRelativeLayout(
    vfs::FileSystem &VFS, llvm::StringRef BinDir, std::string &Path,
    ToolsetLayout &VSLayout, std::string &SDKRoot, std::string &SDKVersion);

// openrt layout as declared by <root>\manifest.json. Relative paths are taken
// against Root; a field that is absent (or not a string) stays empty.
//   "sdk":     {"root": <dir>, "version": <ver>}   -> SDKRoot, SDKVersion
//   "overlay": {"file": <yaml>, "prefix": <dir>}   -> VFSOverlay, VFSPrefix
//              file: VFS overlay that maps API header names onto the files
//              the install ships; prefix: the virtual include directory the
//              overlay's root names (absolute, need not exist on disk).
//   "libmap":  <file>                              -> LibMap
//              library name data for the linker (one "<name>\t<name>" line
//              per mapping; see lld COFF LinkerDriver::loadLibMap) and for
//              the compiler's own default-lib names (readOpenRTLibMap).
//   "toolchain": false                             -> IsToolchain = false
//              the root carries data only (libmap, overlay) and is not a
//              toolchain root: findVCToolChainViaBinRelativeLayout ignores
//              it (e.g. an LLVM install used with another toolchain's libs).
struct OpenRTLayout {
  std::string Root;
  std::string SDKRoot, SDKVersion;
  std::string VFSOverlay, VFSPrefix;
  std::string LibMap;
  bool IsToolchain = true;
};
LLVM_ABI std::optional<OpenRTLayout> findOpenRTLayout(vfs::FileSystem &VFS,
                                                      llvm::StringRef BinDir);

// LIBNAME-OPENRT: reads a libmap file ("<from> <to>" per line, TAB or blanks
// between, '#' comment lines, malformed lines skipped) into Map, keyed by the
// canonical <from> (lower case, ".lib" dropped); the value is <to> as written.
// False if the file cannot be read.
LLVM_ABI bool readOpenRTLibMap(vfs::FileSystem &VFS, llvm::StringRef Path,
                               llvm::StringMap<std::string> &Map);

// Check various environment variables to try and find a toolchain.
LLVM_ABI bool findVCToolChainViaEnvironment(vfs::FileSystem &VFS,
                                            std::string &Path,
                                            ToolsetLayout &VSLayout);

// Query the Setup Config server for installs, then pick the newest version
// and find its default VC toolchain. If `VCToolsVersion` is specified, that
// version is preferred over the latest version.
//
// This is the preferred way to discover new Visual Studios, as they're no
// longer listed in the registry.
LLVM_ABI bool
findVCToolChainViaSetupConfig(vfs::FileSystem &VFS,
                              std::optional<llvm::StringRef> VCToolsVersion,
                              std::string &Path, ToolsetLayout &VSLayout);

// Look in the registry for Visual Studio installs, and use that to get
// a toolchain path. VS2017 and newer don't get added to the registry.
// So if we find something here, we know that it's an older version.
LLVM_ABI bool findVCToolChainViaRegistry(std::string &Path,
                                         ToolsetLayout &VSLayout);

} // namespace llvm

#endif
