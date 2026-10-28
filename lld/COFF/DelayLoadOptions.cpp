//===- DelayLoadOptions.cpp -----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// link.exe /delay:{unload,nobind}. UNLOAD makes DLL.cpp emit an
// UnloadDelayImportTable (a copy of the initial IAT, used by
// __FUnloadDelayLoadedDLL2). NOBIND is accepted as a no-op: lld never emits a
// BoundDelayImportTable.
//
//===----------------------------------------------------------------------===//

#include "COFFLinkerContext.h"
#include "Driver.h"
#include "lld/Common/ErrorHandler.h"

namespace lld::coff {

void LinkerDriver::parseDelay(StringRef arg) {
  if (arg.equals_insensitive("unload"))
    ctx.config.delayUnload = true;
  else if (arg.equals_insensitive("nobind"))
    ctx.config.delayNoBind = true;
  else
    Warn(ctx) << "ignoring unknown argument for /delay: " << arg;
}

} // namespace lld::coff
