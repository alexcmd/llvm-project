// CL-ATTR-AFTER-TYPE: cl 19.44 applies a [[...]] attribute written after the
// type in a declaration, when it is not a type attribute, to the declared
// entity (observed: Fn [[deprecated("use a")]] A; in a union (NVIDIA OMM
// SDK omm.h), int [[deprecated("x")]] g();, a variable and a parameter; C4996
// on each use). Elsewhere it stays an error.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -std=c++17 -fsyntax-only -verify=cl %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -fsyntax-only -verify=std %s

typedef void (*Fn)();
struct S { union { Fn a; Fn [[deprecated("use a")]] A; }; }; // std-error {{'deprecated' attribute cannot be applied to types}} cl-note {{'A' has been explicitly marked deprecated here}}
int f(S &s) { s.A = nullptr; return 0; } // cl-warning {{'A' is deprecated: use a}}
int [[deprecated("x")]] g(); // std-error {{'deprecated' attribute cannot be applied to types}} cl-note {{'g' has been explicitly marked deprecated here}}
int h() { return g(); } // cl-warning {{'g' is deprecated: x}}
int [[deprecated]] v; // std-error {{'deprecated' attribute cannot be applied to types}} cl-note {{'v' has been explicitly marked deprecated here}}
int k() { return v; } // cl-warning {{'v' is deprecated}}
void m(int [[deprecated]] p) { (void)p; } // std-error {{'deprecated' attribute cannot be applied to types}} cl-warning {{'p' is deprecated}} cl-note {{'p' has been explicitly marked deprecated here}}
