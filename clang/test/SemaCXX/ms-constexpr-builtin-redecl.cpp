// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only -verify=std %s

// openrt CL-IDENTITY: cl's <intrin0.inl.h> redeclares a builtin constexpr;
// cl accepts it and so does MSVC compatibility (a declaration only).
// ms-no-diagnostics

extern "C" constexpr void *__builtin_assume_aligned(const void *, unsigned long long, ...) noexcept; // std-error {{constexpr declaration of '__builtin_assume_aligned' follows non-constexpr declaration}} std-note {{previous declaration is here}}

// The builtin keeps its meaning.
int *f(int *p) { return static_cast<int *>(__builtin_assume_aligned(p, 16)); }
