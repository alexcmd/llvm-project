// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -Wunused-value -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -std=c++20 \
// RUN:   -fsyntax-only -Wunused-value -verify=std %s

// openrt UE-NOISE (r32): cl 14.51 reports an unused expression result by
// default only for an operator (C4552, level 1): unary + - ! ~ and a binary
// operator other than assignment, comma, && and ||, unparenthesized. Other
// expression statements are C4555 (off by default) or silent (/Wall).

struct S { int x; };
int g();
void t(int i, bool b, int *p, S s, S *ps, int a[4], float fl) {
  -i;        // ms-warning {{expression result unused}} std-warning {{expression result unused}}
  !b;        // ms-warning {{expression result unused}} std-warning {{expression result unused}}
  ~i;        // ms-warning {{expression result unused}} std-warning {{expression result unused}}
  +i;        // ms-warning {{expression result unused}} std-warning {{expression result unused}}
  i << 1;    // ms-warning {{expression result unused}} std-warning {{expression result unused}}
  i & 1;     // ms-warning {{expression result unused}} std-warning {{expression result unused}}
  g() * 2;   // ms-warning {{expression result unused}} std-warning {{expression result unused}}
  b;         // std-warning {{expression result unused}}
  i;         // std-warning {{expression result unused}}
  fl;        // std-warning {{expression result unused}}
  *p;        // std-warning {{expression result unused}}
  &i;        // std-warning {{expression result unused}}
  1;         // std-warning {{expression result unused}}
  s.x;       // std-warning {{expression result unused}}
  ps->x;     // std-warning {{expression result unused}}
  a[1];      // std-warning {{expression result unused}}
  (i + 1);   // std-warning {{expression result unused}}
  (int)fl;   // std-warning {{expression result unused}}
  i ? 1 : 2; // std-warning {{expression result unused}}
  i && b;    // std-warning {{expression result unused}}
}
