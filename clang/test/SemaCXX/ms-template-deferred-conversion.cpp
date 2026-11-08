// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=none -Wshorten-64-to-32 -Wsign-compare %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify -Wshorten-64-to-32 -Wsign-compare -DINST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=expected,two -Wshorten-64-to-32 -Wsign-compare -DINST -DINST2 %s

// openrt UE-TEMPLATE-NONDEP-DIAG (r28, MovieScene): cl 14.51 reports
// conversion (C4244/C4267) and signed/unsigned (C4018/C4389) warnings in
// template code only at instantiation: an only-included template is clean
// under /we4244. A non-dependent one is reported once however many
// instantiations, a dependent one once per type.
// none-no-diagnostics

typedef unsigned long long u64;
typedef unsigned u32;
void take(u32);
template <class T> void f1(u64 v) { take(v); }               // expected-warning {{implicit conversion loses integer precision}}
template <class T> int f2(int a, unsigned b) { return a < b; } // expected-warning {{comparison of integers of different signs}}
template <class T> void f8(T v) { take(v); }                 // expected-warning {{implicit conversion loses integer precision}} two-warning {{implicit conversion loses integer precision}}
template <class T> struct C { void m(u64 v) { take(v); } };  // expected-warning {{implicit conversion loses integer precision}}

#ifdef INST
void use1() { f1<int>(1); f2<int>(1, 2); f8<u64>(1); C<int>().m(1); } // expected-note 4 {{in instantiation of}}
#endif
#ifdef INST2
void use2() { f1<double>(1); f2<double>(1, 2); f8<long long>(1); C<double>().m(1); } // two-note {{in instantiation of}}
#endif
