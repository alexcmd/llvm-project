// REQUIRES: x86-registered-target
// RUN: %clang_cc1 -ffreestanding -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fms-compatibility-version=19.51 -O0 -emit-llvm -o - %s \
// RUN:   | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fms-compatibility-version=19.51 -O2 -emit-obj -o %t.obj %s

// openrt UE-SSE42-MACRO-INTRIN: cl 14.51 compiles the SSE4.2 string
// intrinsics (macros over target builtins) in a C function without /arch.
// The builtin call goes into an outlined function with the SSE4.2 features
// that the caller reaches through an out-of-line thunk, so only that
// instruction needs the ISA.

#include <nmmintrin.h>

int find_range(const char *p) {
  __m128i ranges = _mm_setr_epi8('a', 'z', 'A', 'Z', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  __m128i word = _mm_loadu_si128((const __m128i *)p);
  if (!_mm_cmpistrc(ranges, word, _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES))
    return -1;
  return _mm_cmpistri(ranges, word, _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES);
}

// CHECK-LABEL: define dso_local i32 @find_range(
// CHECK-NOT: call i32 @llvm.x86.sse42
// CHECK: call i32 @__openrt_ms_builtin.__builtin_ia32_pcmpistric128.openrt.xfeat(ptr
// CHECK: call i32 @__openrt_ms_builtin.__builtin_ia32_pcmpistri128.openrt.xfeat(ptr
// CHECK: define internal i32 @__openrt_ms_builtin.__builtin_ia32_pcmpistric128.openrt.xfeat(ptr %0, ptr %1) unnamed_addr #[[XF:[0-9]+]]
// CHECK: call i32 @llvm.x86.sse42.pcmpistric128(<16 x i8> {{.*}}, <16 x i8> {{.*}}, i8 4)
// CHECK: attributes #0 = {{.*}}"target-features"="+cx8,+mmx,+sse,+sse2,+x87"
// CHECK: attributes #[[XF]] = {{.*}}"target-features"="{{[^"]*}}+sse4.2
