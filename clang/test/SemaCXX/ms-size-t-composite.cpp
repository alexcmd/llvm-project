// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify=sizet -Wshorten-size-t %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify=int64 -Wshorten-int64-to-32 %s

// openrt UE-C4267-CONDITIONAL: cl 14.51 classifies a narrowing by the value's
// type after the usual arithmetic conversions: a ?: or arithmetic result with
// a size_t operand is size_t (C4267), also against ptrdiff_t or a plain
// unsigned long long; a ?: is checked once, not per arm; int op= <64-bit>
// and -size_t are reported too.

typedef unsigned long long size_t;
typedef long long ptrdiff_t;

int t1(bool c, const char *p, const char *q, size_t n) { return c ? (p - q) : n; } // sizet-warning {{implicit conversion loses integer precision}}
int t2(bool c, ptrdiff_t d, size_t n) { return c ? n : d; }                         // sizet-warning {{implicit conversion loses integer precision}}
int t3(size_t n, unsigned long long u) { return n + u; }                            // sizet-warning {{implicit conversion loses integer precision}}
int t4(unsigned long long u, size_t n) { return u * n; }                            // sizet-warning {{implicit conversion loses integer precision}}
int t5(bool c, unsigned long long a, size_t b) { return c ? a : b; }                // sizet-warning {{implicit conversion loses integer precision}}
int t6(size_t n) { int r = 0; r += n; return r; }                                   // sizet-warning {{implicit conversion loses integer precision}}
int t7(ptrdiff_t d) { int r = 0; r += d; return r; }                                // int64-warning {{implicit conversion loses integer precision}}
int t8(size_t n) { return -n; }                                                     // sizet-warning {{implicit conversion loses integer precision}}
int t9(bool c, ptrdiff_t a, unsigned long long b) { return c ? a : b; }             // int64-warning {{implicit conversion loses integer precision}}
int t10(int r) { r += 1LL; return r; } // a constant that fits: silent
