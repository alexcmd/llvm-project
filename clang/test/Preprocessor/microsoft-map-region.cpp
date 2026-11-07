// The implementation map of a large #import'ed type library header is replaced
// by the .tli it names; #pragma implementation_key is accepted.
// RUN: rm -rf %t && split-file %s %t && cd %t
// RUN: %clang_cc1 -fms-extensions -fms-compatibility -fsyntax-only -verify main.cpp
// RUN: %clang_cc1 -fms-extensions -fms-compatibility -E main.cpp | FileCheck %s

//--- lib.tli
#pragma implementation_key(1)
inline int lib::S::get ( ) { return 1; }

//--- main.cpp
// expected-no-diagnostics
namespace lib {
struct S { int get(); };
#pragma start_map_region("lib.tli")
__declspec(implementation_key(1)) int S::get ( );
#pragma stop_map_region
}
int use(lib::S &s) { return s.get(); }

// CHECK-NOT: implementation_key(1)) int S::get
// CHECK: inline int lib::S::get ( ) { return 1; }
