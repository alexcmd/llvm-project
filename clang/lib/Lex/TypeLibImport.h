//===- TypeLibImport.h - MSVC #import type library header generator -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Generates the <name>.tlh/<name>.tli pair that cl's #import writes for a type
// library. Windows only: the type library is read through the public COM
// automation API (LoadTypeLibEx/ITypeLib/ITypeInfo). No LLVM dependencies so
// that the generator can also be built standalone for output comparison.
//===----------------------------------------------------------------------===//

#ifndef CLANG_LEX_TYPELIBIMPORT_H
#define CLANG_LEX_TYPELIBIMPORT_H

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace clang {
namespace typelib {

/// #import attributes (cl spelling in comments).
struct ImportOptions {
  bool RawInterfacesOnly = false;   // raw_interfaces_only
  bool NamedGuids = false;          // named_guids
  bool NoNamespace = false;         // no_namespace
  bool HasRenameNamespace = false;  // rename_namespace("X")
  std::string RenameNamespace;
  bool NoImplementation = false;    // no_implementation
  bool ImplementationOnly = false;  // implementation_only
  bool RawNativeTypes = false;      // raw_native_types
  bool NoSmartPointers = false;     // no_smart_pointers
  bool RawDispinterfaces = false;   // raw_dispinterfaces
  bool NoDualInterfaces = false;    // no_dual_interfaces
  bool NoAutoExclude = false;       // no_auto_exclude
  bool AutoRename = false;          // auto_rename
  bool NoSearchNamespace = false;   // no_search_namespace
  bool EmbeddedIdl = false;         // embedded_idl
  bool HasRawMethodPrefix = false;  // raw_method_prefix("p")
  std::string RawMethodPrefix = "raw_";
  bool HasHighMethodPrefix = false; // high_method_prefix("p")
  std::string HighMethodPrefix;
  std::string RawPropGet = "get_", RawPropPut = "put_",
              RawPropPutRef = "putref_"; // raw_property_prefixes
  std::string HighPropGet = "Get", HighPropPut = "Put",
              HighPropPutRef = "PutRef"; // high_property_prefixes
  std::vector<std::string> Exclude;                          // exclude("a",..)
  std::vector<std::string> Include; // include("a",..): keep auto-excluded
  std::vector<std::pair<std::string, std::string>> Renames; // rename("a","b")
  std::vector<std::string> InjectStatements; // inject_statement("text")
  bool HasLcid = false;  // lcid("409"): locale of a registry (progid:/libid:)
  unsigned long Lcid = 0; // lookup; no effect on the generated text
  unsigned TlbId = 0;    // tlbid(n): n-th type library resource of a DLL
};

struct ImportRequest {
  std::wstring TypeLibPath;  // resolved file (may carry a \N resource index)
  std::string SpelledName;   // as written in the directive, for the banner
  std::string OutputDir;     // directory of the .tlh/.tli (no trailing slash)
  std::string BaseName;      // file stem of the .tlh/.tli
  std::string Banner;        // first banner line ("Created by ...")
  std::string AttrText;      // attribute list as written (reuse check key)
  ImportOptions Options;
  /// auto_rename: names that are macros at the #import get a "__" prefix.
  std::function<bool(const std::string &)> IsMacro;
};

struct ImportResult {
  std::string TlhPath;    // full path of the .tlh (empty if not written)
  std::string TliPath;    // full path of the .tli (empty if not written)
  std::string TlhText;    // file contents as written (UTF-8 BOM, CRLF)
  std::string TliText;
  bool WroteTlh = false, WroteTli = false;
  std::string Error;      // non-empty on failure
};

/// First attribute cl ignores; it also ignores every attribute after it.
struct AttrIssue {
  enum Kind { None, Unknown, ArgCount } K = None; // C4185 / C4186
  std::string Name;
  unsigned Need = 0; // ArgCount: arguments the attribute requires
};

/// Parse the attribute list of the directive (cl spelling, e.g.
/// `raw_interfaces_only rename("a", "b")`). Like cl, an unknown attribute or
/// one with too few arguments stops the scan (reported in Issue).
void parseAttributes(const std::string &Text, ImportOptions &O,
                     AttrIssue &Issue);

/// Generate the header pair; fills Result.TlhText/TliText. Does not write.
bool generate(const ImportRequest &Req, ImportResult &Result);

/// Resolve "progid:X" / "libid:{...}[,maj.min]" through the registry to the
/// type library file; Display receives the text cl puts in the banner.
bool resolveRegistered(const std::string &Spec, unsigned long Lcid,
                       std::string &Path, std::string &Display,
                       std::string &Err);

/// cl's #import step: reuse <OutputDir>\<BaseName>.tlh when its timestamp
/// equals the type library's and its banner line (which carries a hash of
/// the attribute list) matches; otherwise generate and write the .tlh/.tli
/// and stamp them with the type library's timestamp.
bool importTypeLibrary(const ImportRequest &Req, ImportResult &Result);

} // namespace typelib
} // namespace clang

#endif
