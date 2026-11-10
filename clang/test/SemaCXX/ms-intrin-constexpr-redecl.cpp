// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s

// openrt UE-INTRIN-CONSTEXPR-REDECL: the 14.50 <complex> declares
// _mm_fmsub_sd extern "C"; clang's <fmaintrin.h> defines it constexpr
// always_inline. cl takes them as one intrinsic, in either order.

typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));
extern "C" __m128d _mm_fmsub_sd(__m128d, __m128d, __m128d);
# 1 "intrin_like.h" 1 3
extern "C" {
static __inline__ __attribute__((__always_inline__, __target__("fma"))) constexpr __m128d _mm_fmsub_sd(__m128d a, __m128d, __m128d) { return a; }
static __inline__ __attribute__((__always_inline__, __target__("fma"))) constexpr __m128d _mm_fmadd_sd(__m128d a, __m128d, __m128d) { return a; }
}
# 18 "ms-intrin-constexpr-redecl.cpp" 2
extern "C" __m128d _mm_fmadd_sd(__m128d, __m128d, __m128d);
__m128d use(__m128d a) { return _mm_fmsub_sd(a, a, a) + _mm_fmadd_sd(a, a, a); }

int ordinary(int); // expected-note {{previous declaration is here}}
# 1 "ordinary.h" 1 3
__attribute__((always_inline, target("fma"))) inline constexpr int ordinary(int x) { return x; } // expected-error {{constexpr declaration of 'ordinary' follows non-constexpr declaration}}
# 30 "ms-intrin-constexpr-redecl.cpp" 2
