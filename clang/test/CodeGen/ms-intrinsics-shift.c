// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=19.33 -triple x86_64-pc-windows-msvc -O2 -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,X64
// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=19.33 -triple i686-pc-windows-msvc -O2 -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,X86

// __ll_lshift, __ll_rshift and __ull_rshift are inline shifts (there is no
// library definition); the count is masked like the MSVC expansion: by 63 on
// x64, by 31 on x86.

typedef __SIZE_TYPE__ size_t;
#include <intrin.h>

unsigned __int64 test_ll_lshift(unsigned __int64 v, int s) {
  return __ll_lshift(v, s);
}
// CHECK-LABEL: define{{.*}} i64 @test_ll_lshift(
// X64:         and i32 %s, 63
// X86:         and i32 %s, 31
// CHECK-NOT:   call
// CHECK:       shl i64 %v,

__int64 test_ll_rshift(__int64 v, int s) { return __ll_rshift(v, s); }
// CHECK-LABEL: define{{.*}} i64 @test_ll_rshift(
// X64:         and i32 %s, 63
// X86:         and i32 %s, 31
// CHECK-NOT:   call
// CHECK:       ashr i64 %v,

unsigned __int64 test_ull_rshift(unsigned __int64 v, int s) {
  return __ull_rshift(v, s);
}
// CHECK-LABEL: define{{.*}} i64 @test_ull_rshift(
// X64:         and i32 %s, 63
// X86:         and i32 %s, 31
// CHECK-NOT:   call
// CHECK:       lshr i64 %v,
