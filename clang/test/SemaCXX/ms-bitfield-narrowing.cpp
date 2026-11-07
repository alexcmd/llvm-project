// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify -Wshorten-64-to-32 %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++17 -fsyntax-only \
// RUN:   -verify=gnu -Wshorten-64-to-32 %s

// openrt UE-C4244-BITFIELD (Core): cl 14.51 converts a 64-bit bit-field read
// as such to a 32-bit integer without C4244, whatever its width (51, 63, 64),
// as argument, return value, assignment or initializer; an operator applied to
// it (arithmetic, shift, ~, ?:, op=) gives an ordinary 64-bit value, which is
// reported, like a plain unsigned long long.

typedef unsigned long long u64;
typedef unsigned u32;
struct S { u64 a : 51; u64 d : 63; u64 e : 64; long long g : 40; };
void take(u32);

u32 r1(S s) { return s.a; }              // gnu-warning {{implicit conversion loses integer precision}}
u32 r2(S s) { return s.d; }              // gnu-warning {{implicit conversion loses integer precision}}
u32 r3(S s) { return s.e; }              // gnu-warning {{implicit conversion loses integer precision}}
int r4(S s) { return s.g; }              // gnu-warning {{implicit conversion loses integer precision}}
void a1(S s) { take(s.a); }              // gnu-warning {{implicit conversion loses integer precision}}
void a2(S s, u32 &x) { x = s.a; }        // gnu-warning {{implicit conversion loses integer precision}}
void a3(S s) { u32 x = s.a; (void)x; }   // gnu-warning {{implicit conversion loses integer precision}}
u32 o1(S s) { return s.a + 1; }          // expected-warning {{implicit conversion loses integer precision}} gnu-warning {{implicit conversion loses integer precision}}
u32 o2(S s) { return s.a >> 3; }         // expected-warning {{implicit conversion loses integer precision}} gnu-warning {{implicit conversion loses integer precision}}
u32 o3(S s, bool c) { return c ? s.a : s.d; } // expected-warning {{implicit conversion loses integer precision}} gnu-warning 2 {{implicit conversion loses integer precision}}
void o4(S s, u32 &x) { x += s.a; }       // expected-warning {{implicit conversion loses integer precision}}
u32 plain(u64 v) { return v; }           // expected-warning {{implicit conversion loses integer precision}} gnu-warning {{implicit conversion loses integer precision}}
