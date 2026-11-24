// MS-MEMBER-TEMPLATE-LOOKUP. In MS-compat mode 'x.name < ...' with a
// type-dependent x is disambiguated by a token look-ahead when 'name' also
// names a template in the enclosing scope. The look-ahead must stop in front
// of the completion point, also inside a bracket group, and leave it to the
// real parse: member completion after 's.', not an ordinary-name completion.

template <class T> class nf {};
template <class T> struct X { int nf; };
struct S { int alpha; int beta; };
int g1(int);

template <class T> bool plain(const X<T> &x, S s) {
  return x.nf < s.alpha;
}
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -code-completion-at=%s:%(line-2):19 %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsyntax-only -code-completion-at=%s:%(line-3):19 %s -o - | FileCheck %s

template <class T> bool paren(const X<T> &x, S s) {
  return x.nf < g1(s.alpha);
}
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -code-completion-at=%s:%(line-2):22 %s -o - | FileCheck %s

template <class T> bool square(const X<T> &x, S s, int *a) {
  return x.nf < a[s.alpha];
}
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -code-completion-at=%s:%(line-2):21 %s -o - | FileCheck %s

template <class T> bool brace(const X<T> &x, S s) {
  return x.nf < S{s.alpha, 0}.beta;
}
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -code-completion-at=%s:%(line-2):21 %s -o - | FileCheck %s

// CHECK-NOT: COMPLETION: g1
// CHECK: COMPLETION: alpha : [#int#]alpha
// CHECK-NEXT: COMPLETION: beta : [#int#]beta
// CHECK-NOT: COMPLETION: g1
