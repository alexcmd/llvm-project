// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s

// openrt UE-M128-UNION-MEMBERS: cl's __m64/__m128*/__m256*/__m512* are unions
// of element arrays at offset 0 (member set and types measured, cl 14.51).

typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));
typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));
typedef long long __m64 __attribute__((__vector_size__(8), __aligned__(8)));
typedef float __m256 __attribute__((__vector_size__(32), __aligned__(32)));
typedef long long __m512i __attribute__((__vector_size__(64), __aligned__(64)));
template <class A, class B> constexpr bool same = false;
template <class A> constexpr bool same<A, A> = true;

__m128 a; __m128i b; __m64 c; const __m128d d{}; __m256 e; __m512i z;
static_assert(same<decltype(a.m128_f32), float[4]>);
static_assert(same<decltype(d.m128d_f64), double[2]>);
static_assert(same<decltype((a.m128_f32)), float (&)[4]>);
static_assert(same<decltype((a.m128_i8)), char (&)[16]>);
static_assert(same<decltype((a.m128_u64)), unsigned long long (&)[2]>);
static_assert(same<decltype((b.m128i_i32)), int (&)[4]>);
static_assert(same<decltype((b.m128i_u8)), unsigned char (&)[16]>);
static_assert(same<decltype((c.m64_f32)), float (&)[2]>);
static_assert(same<decltype((d.m128d_f64)), const double (&)[2]>);
static_assert(same<decltype((e.m256_f32)), float (&)[8]>);
static_assert(same<decltype((z.m512i_i16)), short (&)[32]>);
float f1(const __m128 &v) { return v.m128_f32[0]; }
int f2(__m128i *p) { p->m128i_i32[1] = 3; return p->m128i_i32[1]; }
long long f3(__m128i v) { return (v + v).m128i_i64[0]; }
__m128 make();
static_assert(same<decltype((make().m128_f32)), float (&&)[4]>);
static_assert(same<decltype((static_cast<__m128 &&>(a).m128_f32)), float (&&)[4]>);
int bad1 = b.m128i_f32[0]; // expected-error {{member reference base type '__m128i' (vector of 2 'long long' values) is not a structure or union}}
double bad2 = e.m256_f64[0]; // expected-error {{member reference base type '__m256' (vector of 8 'float' values) is not a structure or union}}
