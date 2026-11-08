// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fno-ms-clang-identity -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -emit-llvm -o - %s | FileCheck --check-prefix=ID %s

// openrt UE-M128I-INIT: cl's __m128i/__m256i/__m512i are unions with a char
// array first: a braced list gives the bytes (cl 14.51, constant-evaluable,
// brace elision takes up to the byte count). Without the clang identity
// macros, MSVC compatibility does the same for a type spelled so.

typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));
typedef long long __m256i __attribute__((__vector_size__(32), __aligned__(32)));
typedef long long __v2di __attribute__((__vector_size__(16)));
typedef __m128i VectorRegister4Int;

#if !defined(__clang__)
// CHECK: @"?A@@3T__m128i@@B" = {{.*}}constant <2 x i64> <i64 197121, i64 0>
extern const __m128i A;
constexpr __m128i A = {1, 2, 3};
// CHECK: @"?B@@3T__m128i@@B" = {{.*}}constant <2 x i64> splat (i64 -1)
extern const VectorRegister4Int B;
constexpr VectorRegister4Int B = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
// CHECK: @"?C@@3T__m256i@@A" = {{.*}}global <4 x i64> <i64 7, i64 0, i64 0, i64 0>
__m256i C = {7};
struct S { __m128i v; int k; };
// CHECK: @"?G@@3US@@A" = {{.*}}global %struct.S { <2 x i64> <i64 526086, i64 0>, i32 0 }
S G = {6, 7, 8};
#endif
// clang's own spelling keeps its meaning.
// ID: @"?V@@3T__m128i@@A" = {{.*}}global <2 x i64> <i64 1, i64 2>
// CHECK: @"?V@@3T__m128i@@A" = {{.*}}global <2 x i64> <i64 1, i64 2>
__v2di V = {1, 2};

#if !defined(__clang__)
static_assert(((__v2di)A)[0] == 0x030201);
// In a constexpr function, from its parameters.
constexpr __m128i MakeInt(int X, int Y) { return __m128i{(char)X, (char)(X >> 8), (char)Y}; }
static_assert(((__v2di)MakeInt(0x201, 3))[0] == 0x030201);

// CHECK-LABEL: define {{.*}}@"?h@@YA?AT__m128i@@D@Z"
// CHECK: insertelement <16 x i8>
__m128i h(char x) { return __m128i{x, 0x41}; }
#endif
