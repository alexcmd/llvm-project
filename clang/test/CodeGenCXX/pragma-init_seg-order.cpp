// REQUIRES: x86-registered-target
// RUN: %clang_cc1 %s -triple x86_64-pc-windows-msvc -fms-extensions -std=c++17 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple x86_64-pc-windows-msvc -fms-extensions -std=c++17 -S -o - | FileCheck %s --check-prefix=ASM

// init_seg(compiler) initializers of non-comdat variables must not be keyed to
// the variable: a keyed entry goes into an associative .CRT$XCC section, which
// the linker orders after the plain one, so b's initializer would run after c's
// (MSVC STL cout.cpp: filebuf, cout, _Init_cout). Comdat variables keep the key.

int f();
#pragma init_seg(compiler)
static int a = f();
int b = f();
static int c = f();
inline int d = f();
int g() { return d; }

// CHECK: @llvm.global_ctors = appending global [4 x { i32, ptr, ptr }]
// CHECK-SAME: { i32 200, ptr @"??__Ea@@YAXXZ", ptr null }
// CHECK-SAME: { i32 200, ptr @"??__Eb@@YAXXZ", ptr null }
// CHECK-SAME: { i32 200, ptr @"??__Ec@@YAXXZ", ptr null }
// CHECK-SAME: { i32 200, ptr @"??__Ed@@YAXXZ", ptr @"?d@@3HA" }

// ASM:      .section .CRT$XCC,"dr",unique,0
// ASM-NOT:  .section
// ASM:      .quad "??__Ea@@YAXXZ"
// ASM-NOT:  .section
// ASM:      .quad "??__Eb@@YAXXZ"
// ASM-NOT:  .section
// ASM:      .quad "??__Ec@@YAXXZ"
// ASM:      .section .CRT$XCC,"dr",associative,"?d@@3HA",unique,0
// ASM-NOT:  .section
// ASM:      .quad "??__Ed@@YAXXZ"
