// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.51 -fno-ms-clang-identity -ffreestanding \
// RUN:   -target-feature +avx512f -O1 -disable-llvm-passes -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.51 -fno-ms-clang-identity -ffreestanding \
// RUN:   -O1 -disable-llvm-passes -emit-llvm -o - %s -DSSE2 | FileCheck --check-prefix=SSE2 %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.51 -ffreestanding -fsyntax-only -verify %s -DIDENTITY

// openrt CL-IDENTITY: cl's SVML intrinsics are calls of the runtime's
// __vdecl_* routines (cl 14.51 /FA: arguments and results in vector registers
// 0..1, the second result of sincos/divrem in register 1: __vectorcall with an
// HVA return), only without the clang identity macros.

#include <immintrin.h>

#ifdef IDENTITY
// expected-no-diagnostics
#ifdef __SVMLINTRIN_H
#error svml intrinsics with the clang identity macros
#endif
#elif defined(SSE2)
// SSE2-LABEL: define {{.*}}@ceil_sse2
// SSE2: call x86_vectorcallcc <4 x float> @"\01__vdecl_ceilf4"
__m128 ceil_sse2(__m128 a) { return _mm_svml_ceil_ps(a); }
#else
// CHECK-LABEL: define {{.*}}@sin4
// CHECK: call x86_vectorcallcc <4 x float> @"\01__vdecl_sinf4"(<4 x float>
__m128 sin4(__m128 a) { return _mm_sin_ps(a); }
// CHECK-LABEL: define {{.*}}@pow4d
// CHECK: call x86_vectorcallcc <4 x double> @"\01__vdecl_pow4"(<4 x double> {{.*}}, <4 x double>
__m256d pow4d(__m256d a, __m256d b) { return _mm256_pow_pd(a, b); }
// CHECK-LABEL: define {{.*}}@cos16
// CHECK: call x86_vectorcallcc <16 x float> @"\01__vdecl_cosf16"
__m512 cos16(__m512 a) { return _mm512_cos_ps(a); }
// CHECK-LABEL: define {{.*}}@sincos2
// CHECK: call x86_vectorcallcc %struct.__ort_svml_m128dx2 @"\01__vdecl_sincos2"
__m128d sincos2(__m128d *c, __m128d a) { return _mm_sincos_pd(c, a); }
// CHECK-LABEL: define {{.*}}@divrem4
// CHECK: call x86_vectorcallcc %struct.__ort_svml_m128ix2 @"\01__vdecl_i32divrem4"
__m128i divrem4(__m128i *r, __m128i a, __m128i b) { return _mm_idivrem_epi32(r, a, b); }
// CHECK-LABEL: define {{.*}}@div16
// CHECK: call x86_vectorcallcc <2 x i64> @"\01__vdecl_i8div16"
__m128i div16(__m128i a, __m128i b) { return _mm_div_epi8(a, b); }
// CHECK-LABEL: define {{.*}}@trunc8
// CHECK: call <8 x float> @llvm.x86.avx.round.ps.256(<8 x float> {{.*}}, i32 3)
__m256 trunc8(__m256 a) { return _mm256_trunc_ps(a); }
#endif
