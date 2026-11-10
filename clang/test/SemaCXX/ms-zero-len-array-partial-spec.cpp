// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s
// expected-no-diagnostics

// openrt UE-ZERO-LEN-ARRAY-SPEC: cl selects a T[0] partial specialization for
// U[0] (zero-length arrays form in deduction too); measured with cl 14.51.

template <typename T> struct Checker { static T &Func(); };
template <typename T> struct Checker<T[0]> { static T &Func(); };
static_assert(sizeof(Checker<const unsigned char[0]>::Func()));
static_assert(sizeof(Checker<int[0]>::Func()) == sizeof(int));

template <class T> struct C { static constexpr int v = 0; };
template <class T> struct C<T[0]> { static constexpr int v = 1; };
template <class T, int N> struct C<T[N]> { static constexpr int v = 2; };
static_assert(C<int[0]>::v == 1);
static_assert(C<int[3]>::v == 2);
