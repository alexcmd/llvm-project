// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify=sizet -Wshorten-size-t %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify=int64 -Wshorten-int64-to-32 %s

// openrt UE-C4267-TEMPLATE: in a template instantiation a narrowing from a
// template parameter is C4267 (-Wshorten-size-t) when the argument that first
// formed the specialization was spelled size_t (explicitly, or deduced from a
// size_t value or sizeof), else C4244 (-Wshorten-int64-to-32). Measured on
// cl 14.51 /W4: fa<unsigned long long> then fa<size_t> is C4244, the other
// order C4267; a typedef of unsigned long long is C4244.

typedef unsigned long long size_t;
typedef unsigned long long u64;
size_t gn();

template <class T> int fa(T v) { return v; } // int64-warning {{implicit conversion loses integer precision}}
int a1 = fa<unsigned long long>(1); // int64-note {{in instantiation of function template specialization}}
int a2 = fa<size_t>(1);

template <class T> int fb(T v) { return v; } // sizet-warning {{implicit conversion loses integer precision}}
int b1 = fb<size_t>(1); // sizet-note {{in instantiation of function template specialization}}
int b2 = fb<unsigned long long>(1);

template <class T> int fc(T v) { return v; } // sizet-warning {{implicit conversion loses integer precision}}
int c1 = fc(sizeof(int)); // sizet-note {{in instantiation of function template specialization}}

template <class T> int fd(T v) { return v; } // sizet-warning {{implicit conversion loses integer precision}}
int d1 = fd(gn()); // sizet-note {{in instantiation of function template specialization}}

template <class T> int fe(T v) { return v; } // int64-warning {{implicit conversion loses integer precision}}
int e1 = fe<u64>(1); // int64-note {{in instantiation of function template specialization}}

template <class T, class U> int ff(T a, U b) { return a + b; } // sizet-warning {{implicit conversion loses integer precision}}
int f1 = ff<int, size_t>(1, 2); // sizet-note {{in instantiation of function template specialization}}

template <class T> struct S { int g(T v) { return v; } }; // int64-warning {{implicit conversion loses integer precision}}
int s1 = S<unsigned long long>{}.g(1); // int64-note {{in instantiation of member function}}
template <class T> struct R { int g(T v) { return v; } }; // sizet-warning {{implicit conversion loses integer precision}}
int r1 = R<size_t>{}.g(1); // sizet-note {{in instantiation of member function}}

// Like cl, a constant sizeof that fits is not diagnosed.
int k(int *p) { return sizeof(*p); }
