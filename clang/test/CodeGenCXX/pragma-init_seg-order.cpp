// REQUIRES: x86-registered-target
// RUN: %clang_cc1 %s -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN: -std=c++17 -emit-llvm -o - | FileCheck %s -DPRIORITY=200
// RUN: %clang_cc1 %s -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN: -std=c++17 -DLIB_SEG -emit-llvm -o - | FileCheck %s -DPRIORITY=400
// RUN: %clang_cc1 %s -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN: -std=c++17 -S -o - | FileCheck %s --check-prefix=ASM -DSEG=XCC
// RUN: %clang_cc1 %s -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN: -std=c++17 -DLIB_SEG -S -o - | FileCheck %s --check-prefix=ASM \
// RUN: -DSEG=XCL

// Keep init_seg(compiler/lib) initializers of non-COMDAT variables in one
// CRT section in declaration order. Splitting b into an associative section
// would prevent it from appearing between a and c. COMDAT variables retain
// their key for duplicate elimination.

int f();
#ifdef LIB_SEG
#pragma init_seg(lib)
#else
#pragma init_seg(compiler)
#endif
static int a = f();
int b = f();
static int c = f();
inline int d = f();
int g() { return d; }

// CHECK: @llvm.global_ctors = appending global [4 x { i32, ptr, ptr }]
// CHECK-SAME: { i32 [[PRIORITY]], ptr @"??__Ea@@YAXXZ", ptr null }
// CHECK-SAME: { i32 [[PRIORITY]], ptr @"??__Eb@@YAXXZ", ptr null }
// CHECK-SAME: { i32 [[PRIORITY]], ptr @"??__Ec@@YAXXZ", ptr null }
// CHECK-SAME: { i32 [[PRIORITY]], ptr @"??__Ed@@YAXXZ", ptr @"?d@@3HA" }

// ASM:      .section .CRT$[[SEG]],"dr",unique,0
// ASM-NOT:  .section
// ASM:      .quad "??__Ea@@YAXXZ"
// ASM-NOT:  .section
// ASM:      .quad "??__Eb@@YAXXZ"
// ASM-NOT:  .section
// ASM:      .quad "??__Ec@@YAXXZ"
// ASM:      .section .CRT$[[SEG]],"dr",associative,"?d@@3HA",unique,0
// ASM-NOT:  .section
// ASM:      .quad "??__Ed@@YAXXZ"
