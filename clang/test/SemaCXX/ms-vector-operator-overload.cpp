// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -fno-ms-clang-identity -verify=noid %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=id %s

// openrt UE-XM-VECTOR-OPS: cl's __m128 & co. are unions there and take user
// operators (DirectXMath's XMVECTOR operators; cl has no built-in vector
// arithmetic). Without the clang identity macros, the MS vector types do
// too; a user operator wins over clang's built-in vector operation.
// noid-no-diagnostics

typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));

struct Tag {};
Tag operator+(__m128, __m128);          // id-error {{overloaded 'operator+' must have at least one parameter of class or enumeration type}}
__m128 operator*(__m128, float);        // id-error {{overloaded 'operator*' must have at least one parameter of class or enumeration type}}
__m128 &operator+=(__m128 &, __m128);   // id-error {{overloaded 'operator+=' must have at least one parameter of class or enumeration type}}
Tag operator-(__m128);                  // id-error {{overloaded 'operator-' must have at least one parameter of class or enumeration type}}
bool operator==(__m128i, __m128i);      // id-error {{overloaded 'operator==' must have at least one parameter of class or enumeration type}}

template <class T> auto twice(T v) { return v + v; }

void f(__m128 a, __m128 b, __m128i i, __m128i j) {
#if !defined(__clang__)
  Tag t1 = a + b;
  Tag t2 = -a;
  Tag t3 = twice(a);
  __m128 c = a * 2.0f;
  c += a;
  bool e = i == j;
  __m128 d = a - b; // no user operator-: built-in vector subtraction
  (void)t1; (void)t2; (void)t3; (void)e; (void)d;
#endif
}
