// MS-INTRIN-ABI-THUNK: a call to an always_inline intrinsic that needs target
// features the caller lacks goes through a thunk with the intrinsic's
// features, vector arguments and result in memory; the intrinsic is inlined
// into the thunk, the caller keeps its own features and no 256-bit value
// crosses a call in registers. Callers that have the feature inline as usual.

// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=x86_64-pc-windows-msvc \
// RUN:   -fms-compatibility -fms-compatibility-version=19 -fms-extensions \
// RUN:   -emit-llvm -disable-llvm-passes -o - | FileCheck %s --check-prefix=IR
// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=x86_64-pc-windows-msvc \
// RUN:   -fms-compatibility -fms-compatibility-version=19 -fms-extensions \
// RUN:   -O2 -S -o - | FileCheck %s --check-prefix=ASM
// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=i686-pc-windows-msvc \
// RUN:   -fms-compatibility -fms-compatibility-version=19 -fms-extensions \
// RUN:   -O2 -S -o - | FileCheck %s --check-prefix=X86

#include <immintrin.h>

// IR-LABEL: define {{.*}}@"?sum8@@YAHPEBH@Z"
// IR: alloca <4 x i64>
// IR: call void @"?_mm256_loadu_si256@@{{.*}}.openrt.xfeat"(ptr {{.*}}, ptr
// IR: store <4 x i64>
// IR: call void @"?_mm256_add_epi32@@{{.*}}.openrt.xfeat"(ptr {{.*}}, ptr {{.*}}, ptr
// IR: call void @"?_mm256_castsi256_si128@@{{.*}}.openrt.xfeat"(ptr {{.*}}, ptr
// IR-NOT: call {{.*}}@"?_mm256_add_epi32@@YA?AT__m256i@@T1@0@Z"(
// IR: ret i32
int sum8(const int *d) {
  __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(d));
  __m256i b = _mm256_add_epi32(a, a);
  return _mm_cvtsi128_si32(_mm256_castsi256_si128(b));
}

// A caller with the feature: direct call, inlined later; no thunk.
// IR-LABEL: define {{.*}}@"?sum8_avx2@@YAHPEBH@Z"
// IR-NOT: openrt.xfeat
// IR: ret i32
__attribute__((target("avx2"))) int sum8_avx2(const int *d) {
  __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(d));
  __m256i b = _mm256_add_epi32(a, a);
  return _mm_cvtsi128_si32(_mm256_castsi256_si128(b));
}

// The thunk: intrinsic's features, not inlinable, loads/stores through the
// pointers, calls the intrinsic.
// IR: define internal void @"?_mm256_add_epi32@@{{.*}}.openrt.xfeat"(ptr %0, ptr %1, ptr %2) unnamed_addr #[[T:[0-9]+]]
// IR: load <4 x i64>, ptr %1
// IR: load <4 x i64>, ptr %2
// IR: call <4 x i64> @"?_mm256_add_epi32@@YA?AT__m256i@@T1@0@Z"(
// IR: store <4 x i64> {{.*}}, ptr %0
// IR: attributes #[[T]] = { {{.*}}noinline{{.*}}"target-features"="{{[^"]*}}+avx2

// ASM-LABEL: "?sum8@@YAHPEBH@Z":
// ASM-NOT: ymm
// ASM: call{{.*}}openrt.xfeat
// ASM-NOT: ymm
// ASM: retq
// ASM-LABEL: "?sum8_avx2@@YAHPEBH@Z":
// ASM-NOT: call
// ASM: retq
// ASM: "?_mm256_add_epi32@@{{.*}}.openrt.xfeat":
// ASM: vpaddd {{.*}}ymm

// 32-bit x86: the same thunks.
// X86-LABEL: "?sum8@@YAHPBH@Z":
// X86: calll{{.*}}openrt.xfeat
