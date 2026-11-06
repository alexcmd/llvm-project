// MS-compat: x86 intrinsics are callable from a function without the target
// feature (cl accepts them). The call stays a call: the callee keeps its own
// target attribute. Non-MS keeps the diagnostic (NOMS run).
// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=x86_64-pc-windows-msvc -fms-compatibility -fms-compatibility-version=19 -fms-extensions -emit-llvm -o - -Wall -Werror | FileCheck %s
// RUN: %clang_cc1 -x c++ -ffreestanding %s -DNOMS -triple=x86_64-unknown-linux-gnu -emit-llvm -o - -Wno-psabi -verify

#include <immintrin.h>

#ifndef NOMS
// CHECK-LABEL: define {{.*}}f_avx2_cmpeq
// CHECK: call {{.*}}_mm256_cmpeq_epi8
__m256i f_avx2_cmpeq(__m256i a, __m256i b) { return _mm256_cmpeq_epi8(a, b); }

// CHECK-LABEL: define {{.*}}f_avx_add
// CHECK: call {{.*}}_mm256_add_pd
__m256d f_avx_add(__m256d a, __m256d b) { return _mm256_add_pd(a, b); }

// CHECK-LABEL: define {{.*}}f_avx512_add
// CHECK: call {{.*}}_mm512_add_epi32
__m512i f_avx512_add(__m512i a, __m512i b) { return _mm512_add_epi32(a, b); }

// CHECK-LABEL: define {{.*}}f_sse42_cmpestri
// CHECK: call {{.*}}__ms__mm_cmpestri
int f_sse42_cmpestri(__m128i a, __m128i b) { return _mm_cmpestri(a, 3, b, 3, 0x0c); }

// CHECK-LABEL: define {{.*}}f_avx2_shuffle
// CHECK: call {{.*}}__ms__mm256_shuffle_epi32
__m256i f_avx2_shuffle(__m256i a) { return _mm256_shuffle_epi32(a, 0x1b); }
#endif

#ifdef NOMS
__m256i f_nomsE(__m256i a, __m256i b) { return _mm256_cmpeq_epi8(a, b); } // expected-error {{always_inline function '_mm256_cmpeq_epi8' requires target feature 'avx2'}}
#endif
