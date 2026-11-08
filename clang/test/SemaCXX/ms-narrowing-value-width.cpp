// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify -Wshorten-64-to-32 %s

// openrt UE-C4244-VALUE-RANGE: cl 14.51 decides C4244/C4267 on a 64-bit value
// by a structural value width (no folding beyond literals): literal = its
// bits; & min; | ^ + - * ?: max; / left; % right; >> c left - c; << and >> by
// a variable: left; casts min(target, operand); enum its underlying type.
// Measured on 60+ forms; the narrowing is reported only when that width is
// above the target's.

typedef unsigned long long u64;
typedef long long i64;
typedef unsigned u32;
typedef int i32;
typedef unsigned char u8;
enum class E8 : u8 { A };
enum class E64 : u64 { A };
struct A { int &operator[](i32 i); };

u32 ok1(u64 x) { return x & 0xFFFF; }
u32 ok2(u64 x, u32 n) { return x & ((1ull << n) - 1); }
u32 ok3(u64 x) { return x >> 32; }
u32 ok4(u64 x, u32 d) { return x % d; }
u32 ok5(u64 x, u32 n) { return n * (x >> 63); }
void ok6(u32 &f, u64 x, u32 n) { f |= n * (x >> 63); }
int ok7(A &a, E8 e) { return a[static_cast<i64>(e)]; }
u32 ok8(u32 v) { return (u64)v << 4; }
u32 ok9(u64 x) { return x & ~0ull; }

u32 w1(u64 x) { return x & 0x1FFFFFFFFull; }       // expected-warning {{implicit conversion loses integer precision}}
u32 w2(u64 x) { return x >> 31; }                  // expected-warning {{implicit conversion loses integer precision}}
u32 w3(u64 x) { return x / 0x100000000ull; }       // expected-warning {{implicit conversion loses integer precision}}
u32 w4(u64 x, u64 d) { return x % d; }             // expected-warning {{implicit conversion loses integer precision}}
u32 w5(u64 x) { return x - (x & 0xFF); }           // expected-warning {{implicit conversion loses integer precision}}
int w6(A &a, i64 i) { return a[i]; }               // expected-warning {{implicit conversion loses integer precision}}
int w7(A &a, E64 e) { return a[static_cast<i64>(e)]; } // expected-warning {{implicit conversion loses integer precision}}
void w8(u32 &f, u64 x) { f += x; }                 // expected-warning {{implicit conversion loses integer precision}}
