// CL-ATTR-AFTER-TYPE: cl 19.44 applies a [[...]] attribute written after the
// type in a declaration, when it is not a type attribute, to the declared
// entity (observed: Fn [[deprecated("use a")]] A; in a union (NVIDIA OMM
// SDK omm.h), int [[deprecated("x")]] g();, a variable and a parameter; C4996
// on each use). Elsewhere it stays an error.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -std=c++17 -fsyntax-only -verify=cl %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -fsyntax-only -verify=std %s
// The rule needs MS compatibility as well as -fms-cl-rules.
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fms-cl-rules -std=c++17 -fsyntax-only -verify=std %s

typedef void (*Fn)();
struct S { union { Fn a; Fn [[deprecated("use a")]] A; }; }; // std-error {{'deprecated' attribute cannot be applied to types}} cl-note {{'A' has been explicitly marked deprecated here}}
int f(S &s) { s.A = nullptr; return 0; } // cl-warning {{'A' is deprecated: use a}}
int [[deprecated("x")]] g(); // std-error {{'deprecated' attribute cannot be applied to types}} cl-note {{'g' has been explicitly marked deprecated here}}
int h() { return g(); } // cl-warning {{'g' is deprecated: x}}
int [[deprecated]] v; // std-error {{'deprecated' attribute cannot be applied to types}} cl-note {{'v' has been explicitly marked deprecated here}}
int k() { return v; } // cl-warning {{'v' is deprecated}}
void m(int [[deprecated]] p) { (void)p; } // std-error {{'deprecated' attribute cannot be applied to types}} cl-warning {{'p' is deprecated}} cl-note {{'p' has been explicitly marked deprecated here}}

// Only [[deprecated]] moves to the entity; [[nodiscard]] and [[maybe_unused]]
// are ignored silently; the others stay errors (cl: noreturn C3829,
// fallthrough C2416, likely C7543). An alias gets the deprecation (cl C4996 'T').
int [[nodiscard]] nd(); // std-error {{'nodiscard' attribute cannot be applied to types}}
void usend() { nd(); }
int [[maybe_unused]] mu; // std-error {{'maybe_unused' attribute cannot be applied to types}}
void [[noreturn]] nr(); // std-error {{'noreturn' attribute cannot be applied to types}} cl-error {{'noreturn' attribute cannot be applied to types}}
int [[fallthrough]] ft; // std-error {{'fallthrough' attribute cannot be applied to types}} cl-error {{'fallthrough' attribute cannot be applied to types}}
using T = int [[deprecated("t")]]; // std-error {{'deprecated' attribute cannot be applied to types}} cl-note {{'T' has been explicitly marked deprecated here}}
T tv; // cl-warning {{'T' is deprecated: t}}
int sz = sizeof(int [[deprecated]]); // std-error {{'deprecated' attribute cannot be applied to types}} (cl: silent)
