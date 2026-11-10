// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -std=c++20 -fsyntax-only -verify %s
typedef float __m128 __attribute__((vector_size(16)));
typedef long long __m128i __attribute__((vector_size(16)));
typedef float ordinary __attribute__((vector_size(16)));
__m128 extra = {{1, 2, 3, 4, 5}}; // expected-error {{excess elements}}
__m128i extra_bytes = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}}; // expected-error {{excess elements}}
ordinary unchanged = {{1, 2, 3, 4}}; // expected-error {{excess elements in scalar initializer}}
