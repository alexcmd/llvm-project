// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s
// expected-no-diagnostics

// openrt CL-IDENTITY: cl's __is_assignable_no_precondition_check (behind its
// _IS_ASSIGNABLE_NOCHECK_SUPPORTED): __is_assignable, with an incomplete type
// giving false instead of an error. Values measured with cl 14.51.

struct S {}; struct I; struct D { D &operator=(const D &) = delete; };
struct C { C &operator=(int); };
static_assert(__is_assignable_no_precondition_check(S &, const S &));
static_assert(__is_assignable_no_precondition_check(S, const S &));
static_assert(__is_assignable_no_precondition_check(int &, double));
static_assert(!__is_assignable_no_precondition_check(int, int));
static_assert(!__is_assignable_no_precondition_check(D &, const D &));
static_assert(__is_assignable_no_precondition_check(C &, int));
static_assert(!__is_assignable_no_precondition_check(C &, void *));
static_assert(!__is_assignable_no_precondition_check(void, int));
static_assert(!__is_assignable_no_precondition_check(I &, I &));
static_assert(!__is_assignable_no_precondition_check(I, I));
static_assert(!__is_assignable_no_precondition_check(int[2], int[2]));
template <class T, class U> constexpr bool A = __is_assignable_no_precondition_check(T, U);
static_assert(A<S &, S> && !A<I &, int>);
