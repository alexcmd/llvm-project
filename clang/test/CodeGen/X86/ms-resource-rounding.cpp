// REQUIRES: x86-registered-target
// Use %clang -cc1 rather than %clang_cc1: the latter adds the resource path as
// a builtin system include. This regression needs an ordinary -I resource path,
// as used by an isolated MSVC-compatible header stack with -nobuiltininc.
// RUN: %clang -cc1 -x c++ -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.50 -std=c++20 -ffreestanding \
// RUN:   -I "%clang-resource-dir/include" -fno-ms-clang-identity -DHIDDEN=1 \
// RUN:   -O2 -emit-llvm -o - %s | FileCheck %s --check-prefixes=COMMON,BASE
// RUN: %clang -cc1 -x c++ -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.50 -std=c++20 -ffreestanding \
// RUN:   -I "%clang-resource-dir/include" -fno-ms-clang-identity -DHIDDEN=1 \
// RUN:   -target-feature +avx2 -O2 -emit-llvm -o - %s | FileCheck %s --check-prefixes=COMMON,AVX
// RUN: %clang -cc1 -x c++ -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.50 -std=c++20 -ffreestanding \
// RUN:   -I "%clang-resource-dir/include" -DHIDDEN=0 \
// RUN:   -target-feature +avx2 -O2 -emit-llvm -o - %s | FileCheck %s --check-prefixes=COMMON,AVX
// RUN: not %clang -cc1 -x c++ -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.50 -std=c++20 -ffreestanding \
// RUN:   -I "%clang-resource-dir/include" -DHIDDEN=0 \
// RUN:   -O2 -emit-llvm -o %t %s 2>&1 | FileCheck %s --check-prefix=DIAG

#if HIDDEN && defined(__clang__)
#error Expected hidden Clang identity
#elif !HIDDEN && !defined(__clang__)
#error Expected visible Clang identity
#endif

extern "C" {
#include <smmintrin.h>
}

extern "C" {
// COMMON-LABEL: define {{.*}}@load64(
// COMMON: load i64, ptr {{.*}}, align 1
// COMMON: insertelement <2 x i64> <i64 poison, i64 0>, i64 {{.*}}, i64 0
// COMMON: ret <2 x i64>
__m128i load64(const void *p) {
  return _mm_loadl_epi64((const __m128i_u *)p);
}

// BASE-LABEL: define {{.*}}@round_pd(
// BASE: call {{.*}}__ms__mm_round_pd
// AVX-LABEL: define {{.*}}@round_pd(
// AVX-NOT: __ms__mm_round_
// AVX: call {{.*}}@llvm.x86.sse41.round.pd({{.*}}i32 0)
// AVX: ret
__m128d round_pd(__m128d x) { return _mm_round_pd(x, 0); }

// BASE-LABEL: define {{.*}}@round_ps(
// BASE: call {{.*}}__ms__mm_round_ps
// AVX-LABEL: define {{.*}}@round_ps(
// AVX-NOT: __ms__mm_round_
// AVX: call {{.*}}@llvm.x86.sse41.round.ps({{.*}}i32 0)
// AVX: ret
__m128 round_ps(__m128 x) { return _mm_round_ps(x, 0); }

// BASE-LABEL: define {{.*}}@round_sd(
// BASE: call {{.*}}__ms__mm_round_sd
// AVX-LABEL: define {{.*}}@round_sd(
// AVX-NOT: __ms__mm_round_
// AVX: call {{.*}}@llvm.x86.sse41.round.sd({{.*}}i32 0)
// AVX: ret
__m128d round_sd(__m128d x, __m128d y) { return _mm_round_sd(x, y, 0); }

// BASE-LABEL: define {{.*}}@round_ss(
// BASE: call {{.*}}__ms__mm_round_ss
// AVX-LABEL: define {{.*}}@round_ss(
// AVX-NOT: __ms__mm_round_
// AVX: call {{.*}}@llvm.x86.sse41.round.ss({{.*}}i32 0)
// AVX: ret
__m128 round_ss(__m128 x, __m128 y) { return _mm_round_ss(x, y, 0); }
}

// The compatibility helpers must retain their feature target and real work.
// BASE: attributes #{{[0-9]+}} = { {{.*}}"target-features"="{{[^"]*}}+sse4.1{{[^"]*}}"

// DIAG-NOT: narrowing
// DIAG: error: always_inline function '__ms__mm_round_pd' requires target feature 'sse4.1'
// DIAG: error: always_inline function '__ms__mm_round_ps' requires target feature 'sse4.1'
// DIAG: error: always_inline function '__ms__mm_round_sd' requires target feature 'sse4.1'
// DIAG: error: always_inline function '__ms__mm_round_ss' requires target feature 'sse4.1'
