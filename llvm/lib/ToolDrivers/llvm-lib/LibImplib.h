//===- LibImplib.h - link /LIB /DEF import library from objects -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// LIB-DEF-FROMOBJ: link.exe /LIB with /DEF (bare or /DEF:file) builds an
// import library and an exports object (.exp) instead of an archive. The
// exports are the /EXPORT directives of the input objects, the .def EXPORTS
// and the command line /EXPORT options (probed with link 14.51.36231).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TOOLDRIVERS_LLVM_LIB_LIBIMPLIB_H
#define LLVM_LIB_TOOLDRIVERS_LLVM_LIB_LIBIMPLIB_H

#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/Object/COFFImportFile.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBufferRef.h"
#include <vector>

namespace llvm {
namespace libimplib {

/// Parses a link.exe export spec
/// "entry[=internal][,@ordinal[,NONAME]][,DATA][,PRIVATE]" (the part after
/// "/EXPORT:"). Name is the internal name, ExtName the entry name when the
/// two differ (as COFFModuleDefinition does).
Error parseExportSpec(StringRef Spec, object::COFFShortExport &E);

/// Appends the /EXPORT (and -export) directives of a COFF object's .drectve
/// sections. Non-COFF inputs (archives, bitcode) contribute nothing.
Error collectDirectiveExports(MemoryBufferRef MB,
                              std::vector<object::COFFShortExport> &Exports);

/// Drops duplicate entry names (first one wins), sorts by entry name and
/// assigns ordinals to the exports that have none, as link.exe does.
void finalizeExports(std::vector<object::COFFShortExport> &Exports);

/// Writes the exports object: a COFF object whose .edata section holds the
/// export directory, address, name pointer and ordinal tables and the names,
/// with ADDR32NB relocations against the exported (undefined) symbols. Same
/// layout as link.exe's .exp.
Error writeExportsObject(StringRef Path, StringRef DllName,
                         ArrayRef<object::COFFShortExport> Exports,
                         COFF::MachineTypes Machine);

} // namespace libimplib
} // namespace llvm

#endif
