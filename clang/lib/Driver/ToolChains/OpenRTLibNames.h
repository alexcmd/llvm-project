//===--- OpenRTLibNames.h - Default-lib names of the openrt runtime --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// LIBNAME-OPENRT: the library names the driver itself asks for on the MSVC
// target (the C runtime flavour from /MD /MDd /MT /MTd and the old-names
// library). One table, openrt names only. An install whose manifest.json
// names a libmap can map these names onto other files (the same data lld-link
// applies to library names in objects and on the command line).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_OPENRTLIBNAMES_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_OPENRTLIBNAMES_H

#include "clang/Driver/Driver.h"
#include "clang/Driver/ToolChain.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/WindowsDriver/MSVCPaths.h"
#include <string>

namespace clang {
namespace driver {
namespace openrt {

enum class DefaultLib { RuntimeMD, RuntimeMDd, RuntimeMT, RuntimeMTd, OldNames };

struct DefaultLibName {
  DefaultLib Lib;
  const char *Name;
};

inline constexpr DefaultLibName DefaultLibNames[] = {
    {DefaultLib::RuntimeMD, "openruntime_md"},
    {DefaultLib::RuntimeMDd, "openruntime_mdd"},
    {DefaultLib::RuntimeMT, "openruntime_mt"},
    {DefaultLib::RuntimeMTd, "openruntime_mtd"},
    {DefaultLib::OldNames, "openoldnames"},
};

/// The table name of Lib.
inline const char *defaultLibTableName(DefaultLib Lib) {
  for (const DefaultLibName &E : DefaultLibNames)
    if (E.Lib == Lib)
      return E.Name;
  return "";
}

/// The name the driver records for Lib: the table name, or what the libmap of
/// the openrt layout around the driver maps it to.
inline std::string defaultLibName(const ToolChain &TC, DefaultLib Lib) {
  std::string Name = defaultLibTableName(Lib);
  std::optional<llvm::OpenRTLayout> L =
      llvm::findOpenRTLayout(TC.getVFS(), TC.getDriver().Dir);
  if (!L || L->LibMap.empty())
    return Name;
  llvm::StringMap<std::string> Map;
  if (!llvm::readOpenRTLibMap(TC.getVFS(), L->LibMap, Map))
    return Name;
  auto It = Map.find(Name);
  if (It == Map.end())
    return Name;
  return It->second;
}

} // namespace openrt
} // namespace driver
} // namespace clang

#endif
