// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -x c++-header -emit-pch -o %t %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -include-pch %t -emit-llvm -o - %s | FileCheck %s
#ifndef MS_SIMD_MEMBERS_PCH
#define MS_SIMD_MEMBERS_PCH
typedef float __m128 __attribute__((vector_size(16)));
inline float read(const __m128 &v) { return v.m128_f32[1]; }
inline void write(__m128 &v, float x) { v.m128_f32[1] = x; }
#else
// CHECK-LABEL: define {{.*}}@"?use@@
// CHECK: call {{.*}}@"?write@@
// CHECK: call {{.*}}@"?read@@
float use(__m128 &v, float x) { write(v, x); return read(v); }
#endif
