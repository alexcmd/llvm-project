// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -std=c++20 -emit-llvm -o - %s | FileCheck %s
typedef float __m128 __attribute__((vector_size(16)));
typedef long long __m128i __attribute__((vector_size(16)));
typedef __m128 Alias;
// CHECK: @"?values@@{{.*}}constant <4 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00>
extern const Alias values;
constexpr Alias values = {{1, 2, 3, 4}};
// CHECK: @"?bytes@@{{.*}}constant <2 x i64> <i64 67305985, i64 0>
extern const __m128i bytes;
constexpr __m128i bytes = {{1, 2, 3, 4}};
// CHECK: @"?empty@@{{.*}}constant <4 x float> zeroinitializer
extern const __m128 empty;
constexpr __m128 empty = {{}};
static_assert(values[3] == 4);
static_assert(bytes[0] == 0x04030201);
