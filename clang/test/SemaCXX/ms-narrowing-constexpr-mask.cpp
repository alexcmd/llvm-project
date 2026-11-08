// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++17 -fsyntax-only -verify -Wshorten-64-to-32 %s

// openrt UE-MASK-CONSTEXPR (r26, Core WaitingQueue): in cl 14.51's C4244 value
// width any integral constant expression counts by the bits of its value read
// as a signed number: constexpr and const variables, enumerators, static
// constexpr members, template arguments, constant expressions written inline.
// ~0ull is -1 (one bit); 0xFF00000000 is 40 bits; 0xFFFFFFFF00000000 33.

typedef unsigned long long u64;
typedef unsigned u32;
typedef int i32;
constexpr u64 K14 = (1ull << 14) - 1;
const u64 C14 = (1ull << 14) - 1;
constexpr u64 K40 = (1ull << 40) - 1;
constexpr u64 KALL = ~0ull;
enum : u64 { E14 = 0x3FFF, E40 = 0xFFFFFFFFFF };
struct S { static constexpr u64 M14 = (1ull << 14) - 1; static constexpr u64 M40 = (1ull << 40) - 1; };
struct Arr { int &operator[](i32); };

int r26(Arr &a, u64 st) { return a[st & S::M14]; }
u32 ok1(u64 x) { return x & K14; }
u32 ok2(u64 x) { return x & C14; }
u32 ok3(u64 x) { return x & KALL; }
u32 ok4(u64 x) { return x & E14; }
u32 ok5(u64 x) { return x & ((1ull << 14) - 1); }
u32 ok6(u64 x) { return x & ~0xFFFFull; }
u32 ok7(u64 x) { return x % K14; }

u32 w1(u64 x) { return x & K40; }                  // expected-warning {{implicit conversion loses integer precision}}
u32 w2(u64 x) { return x & E40; }                  // expected-warning {{implicit conversion loses integer precision}}
u32 w3(u64 x) { return x & S::M40; }               // expected-warning {{implicit conversion loses integer precision}}
u32 w4(u64 x) { return x & ((1ull << 40) - 1); }   // expected-warning {{implicit conversion loses integer precision}}
u32 w5(u64 x) { return x & (0xFFull << 32); }      // expected-warning {{implicit conversion loses integer precision}}
u32 w6(u64 x) { return x & 0xFFFFFFFF00000000ull; } // expected-warning {{implicit conversion loses integer precision}}

template <u64 M> u32 t1(u64 x) { return x & M; } // expected-warning {{implicit conversion loses integer precision}}
u32 t2(u64 x) { return t1<0x3FFF>(x) + t1<0xFFFFFFFFFFull>(x); } // expected-note {{in instantiation of function template specialization}}
