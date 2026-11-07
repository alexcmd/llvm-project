// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify=sizet -Wshorten-size-t %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify=int64 -Wshorten-int64-to-32 %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify=sizet,int64 -Wshorten-64-to-32 %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++17 -fsyntax-only \
// RUN:   -verify=gnu -Wshorten-64-to-32 %s

// openrt UE-C4244-SHORTEN: in MSVC-compatible mode a narrowing from a type
// written as size_t (to any narrower integer) is -Wshorten-size-t (cl C4267),
// from any other 64-bit integer -Wshorten-int64-to-32 (cl C4244), as cl 14.51
// numbers them. -Wshorten-64-to-32 covers both; without MSVC compatibility
// it is one warning as upstream.
// Like cl 14.51, a constant sizeof that fits is not diagnosed (measured).

typedef unsigned long long size_t;
typedef unsigned long long uint64_t;
typedef size_t mysize;

int a(size_t n) { return n; }     // sizet-warning {{implicit conversion loses integer precision}} gnu-warning {{implicit conversion loses integer precision}}
int b(uint64_t v) { return v; }   // int64-warning {{implicit conversion loses integer precision}} gnu-warning {{implicit conversion loses integer precision}}
int c(mysize v) { return v; }     // sizet-warning {{implicit conversion loses integer precision}} gnu-warning {{implicit conversion loses integer precision}}
short e(size_t n) { return n; }   // sizet-warning {{implicit conversion loses integer precision}}
int f(long long v) { return v; }  // int64-warning {{implicit conversion loses integer precision}} gnu-warning {{implicit conversion loses integer precision}}
