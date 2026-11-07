// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-compatibility-version=19.51 -std=c++20 -fsyntax-only -verify %s

// openrt UE-C4838-CONSTEXPR: cl 14.51 reports no narrowing in the aggregate
// initializer of a constexpr variable (also inexact, out-of-range and
// overflowing constants); a non-constexpr variable gets C4838. The value
// change warnings stay (cl: C4244/C4309).

struct C { char c; int i; float f; };
constexpr C a{0, 1.5, 0}; // expected-warning {{changes value from 1.5 to 1}}
constexpr C b{0, 1.0, 0};
constexpr C d{0, 0, 1e300};
constexpr C f{300, 0, 0}; // expected-warning {{changes value from 300 to 44}}
void fn() { constexpr C l{0, 3.0, 0}; (void)l; }
const C k{0, 2.0, 0}; // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
C g{0, 2.0, 0};       // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
