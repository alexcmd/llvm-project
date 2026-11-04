// RUN: %clang_cc1 -fsyntax-only -fms-compatibility -fms-extensions -std=c++17 -verify %s
// RUN: %clang_cc1 -fsyntax-only -fms-compatibility -fms-extensions -std=c++17 -Wmicrosoft-template -verify=warn %s
// RUN: %clang_cc1 -fsyntax-only -std=c++17 -verify=nonms %s

// cl accepts an out-of-class definition of a member of an implicit or partial
// class template specialization without 'template<>' (C5259, /Wall only).
// Nested class definitions stay errors (cl C2906). Non-MS mode is unchanged.

template <class T, class U = int> struct S {
  static int id;
  static const int cid;
  void f();
  S();
  struct N { int x; };
};

template <class U> struct S<char, U> {
  static int pid;
};

int S<int>::id = 1; // warn-warning {{explicit specialization requires 'template<>'}} nonms-error {{template specialization requires 'template<>'}}
const int S<int, char>::cid = 3; // warn-warning {{explicit specialization requires 'template<>'}} nonms-error {{template specialization requires 'template<>'}}
void S<int>::f() {} // warn-warning {{explicit specialization requires 'template<>'}} nonms-error {{template specialization requires 'template<>'}}
S<int>::S() {} // warn-warning {{explicit specialization requires 'template<>'}} nonms-error {{template specialization requires 'template<>'}}
int S<char>::pid = 2; // warn-warning {{explicit specialization requires 'template<>'}} nonms-error {{template specialization requires 'template<>'}}

struct S<int>::N { int x; }; // expected-error {{template specialization requires 'template<>'}} warn-error {{template specialization requires 'template<>'}} nonms-error {{template specialization requires 'template<>'}}

// Real explicit specialization: no diagnostic in any mode.
template <> struct S<long> {
  static int id;
  void f();
};
int S<long>::id = 4;
void S<long>::f() {}

int use_it() { return S<int>::id + S<long>::id; }
