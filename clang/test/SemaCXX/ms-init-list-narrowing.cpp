// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.51 \
// RUN:   -std=c++20 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.51 \
// RUN:   -std=c++20 -fsyntax-only -verify=off -Wno-microsoft-init-list-narrowing %s

// openrt UE-NARROWING-C4838: as cl 14.51, narrowing in an aggregate
// initializer of a variable (direct or copy list-init, nested, any storage)
// is a warning (C4838, /wd4838 = -Wno-microsoft-init-list-narrowing); scalar
// list-init, T{...} prvalues, return, arguments, new, mem-initializers and
// default member initializers stay errors (C2397).

struct R { long long a; int b; };
struct H { R r; };
void take(R);
double gd;

R ret(unsigned v) { return {0, v}; } // expected-error {{cannot be narrowed}} expected-note {{insert an explicit cast}} off-error {{cannot be narrowed}} off-note {{insert an explicit cast}}
void f(unsigned long long u, unsigned v, double d) {
  R x1{0, v};       // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
  R x2 = {0, v};    // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
  H h{{u, 0}};      // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
  int a1[] = {0, v}; // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
  static R s{0, d}; // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
  struct C { char c; } c1{300}; // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}} expected-warning {{changes value}} off-warning {{changes value}}
  R x3 = R{0, v};   // expected-error {{cannot be narrowed}} expected-note {{insert an explicit cast}} off-error {{cannot be narrowed}} off-note {{insert an explicit cast}}
  take({0, v});     // expected-error {{cannot be narrowed}} expected-note {{insert an explicit cast}} off-error {{cannot be narrowed}} off-note {{insert an explicit cast}}
  int i1{v};        // expected-error {{cannot be narrowed}} expected-note {{insert an explicit cast}} off-error {{cannot be narrowed}} off-note {{insert an explicit cast}}
  auto *p = new R{0, v}; // expected-error {{cannot be narrowed}} expected-note {{insert an explicit cast}} off-error {{cannot be narrowed}} off-note {{insert an explicit cast}}
  (void)x1; (void)x2; (void)h; (void)a1; (void)s; (void)c1; (void)x3; (void)i1; (void)p;
}
R g{0, gd};         // expected-warning {{cannot be narrowed}} expected-note {{insert an explicit cast}}
struct M { int m; M(unsigned v) : m{v} {} }; // expected-error {{cannot be narrowed}} expected-note {{insert an explicit cast}} off-error {{cannot be narrowed}} off-note {{insert an explicit cast}}
struct X { R r{0, gd}; }; // expected-error {{cannot be narrowed}} expected-note {{insert an explicit cast}} off-error {{cannot be narrowed}} off-note {{insert an explicit cast}}
