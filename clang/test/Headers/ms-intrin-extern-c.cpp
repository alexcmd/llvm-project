// REQUIRES: x86-registered-target
// RUN: %clang_cc1 -ffreestanding -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fms-compatibility-version=19.51 -fsyntax-only -verify %s
// RUN: %clang_cc1 -ffreestanding -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fms-compatibility-version=19.51 -target-feature +avx2 \
// RUN:   -fsyntax-only -verify %s

// openrt UE-INTRIN-EXTERNC: the x86 intrinsic headers work inside an enclosing
// extern "C" (Eigen style), as with cl: their MS-compat C++ templates are in
// extern "C++".
// expected-no-diagnostics

extern "C" {
#include <immintrin.h>
}
int a(__m128i v) { return _mm_extract_epi32(v, 1); }
__m128i b(__m128i x, __m128i y) { return _mm_cmpestrm(x, 3, y, 4, 0); }
__m256 c(__m256 x, __m256 y) { return _mm256_permute2f128_ps(x, y, 1); }
__m256i d(__m256i x, __m256i y) { return _mm256_blend_epi32(x, y, 3); }
