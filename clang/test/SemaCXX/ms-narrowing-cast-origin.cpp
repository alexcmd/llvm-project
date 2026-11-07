// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify -Wshorten-64-to-32 %s

// openrt UE-C4244-CAST-ORIGIN (TraceServices): cl 14.51 traces a 64-bit value
// back to its origin through casts and arithmetic: a cast gives min(its width,
// the operand's), arithmetic and ?: the max of their operands, anything else
// its type's width (double 64, float 32). A value of origin <= 32 bits
// narrowed to 32 bits gets no C4244; one of 64-bit origin does.

typedef unsigned long long u64;
typedef unsigned u32;
typedef long long i64;
struct S { u32 f; };

void ok1(S &s, u32 a, u32 b) { s.f = (u64)((double)a / b); }
void ok2(S &s, u32 a) { s.f = (u64)a; }
void ok3(S &s, float x) { s.f = (u64)x; }
void ok4(S &s, int a) { s.f = (u64)a; }
void ok5(S &s, u32 a, u32 b) { s.f = (u64)a * b; }
void ok6(S &s, u64 a) { s.f = (u64)(u32)a; }
void ok7(S &s, u32 a) { s.f = (u64)(a / 2.0f); }
u32 ok8(u32 a, u32 b) { return (u64)((double)a / b); }

void w1(S &s, u64 a, u64 b) { s.f = (u64)((double)a / b); } // expected-warning {{implicit conversion loses integer precision}}
void w2(S &s, double d) { s.f = (u64)d; }                   // expected-warning {{implicit conversion loses integer precision}}
void w3(S &s, u32 a) { s.f = (u64)(a + 1.0); }              // expected-warning {{implicit conversion loses integer precision}}
void w4(S &s, i64 a) { s.f = (u64)a; }                      // expected-warning {{implicit conversion loses integer precision}}
void w5(S &s, double d) { s.f = (u64)(d / 2); }             // expected-warning {{implicit conversion loses integer precision}}
u32 plain(u64 v) { return v; }                              // expected-warning {{implicit conversion loses integer precision}}
