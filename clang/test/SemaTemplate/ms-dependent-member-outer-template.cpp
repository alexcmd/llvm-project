// RUN: %clang_cc1 -std=c++14 -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -std=c++23 -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++14 -fms-extensions -fms-compatibility -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions -fms-compatibility -fdelayed-template-parsing -fsyntax-only -verify=ms %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /Zs /std:c++17 -Xclang -verify=ms -- %s

// ms-no-diagnostics

// MS-MEMBER-TEMPLATE-LOOKUP. x.name < ... with a type-dependent x: before
// C++23, [basic.lookup.classref] also looks name up in the enclosing scope, and
// a class template found there turns '<' into a template-argument list. cl
// accepts every case below in all /std and /permissive modes (nlohmann/json
// unit-regression1). In MS compatibility mode the enclosing-scope class
// template is kept only when a '>' closes the would-be argument list and is
// followed by '::' or '('.

template <typename T> class number_float {};

template <typename T> struct value {
  double number_float;
  double arr[2];
};

double g(double, double);

template <typename T> struct holder {
  value<T> v;
  bool dot(const holder &o) const {
    return v.number_float < o.v.number_float; // std-error {{expected '>'}} std-note {{to match this '<'}} std-error {{expected unqualified-id}}
  }
  bool arrow(const holder *o) const {
    return this->v.number_float < o->v.number_float; // std-error {{expected '>'}} std-note {{to match this '<'}} std-error {{expected unqualified-id}}
  }
  // A '>' nested in parentheses or brackets does not close the list.
  bool paren(double a, double b) const {
    return v.number_float < (a > b); // std-error {{expected '>'}} std-note {{to match this '<'}} std-error {{expected unqualified-id}}
  }
  bool call(double a, double b) const {
    return v.number_float < g(a > b, b) && v.arr[0 > 1] > 0; // std-error {{expected ';' after return statement}} std-error {{no member named 'number_float' in 'value<int>'}}
  }
};

// Member found through a dependent base: this->name.
template <typename T> struct base { double number_float; };
template <typename T> struct derived : base<T> {
  bool lt(const derived &o) const {
    return this->number_float < o.number_float; // std-error {{expected '>'}} std-note {{to match this '<'}} std-error {{expected unqualified-id}}
  }
};

// Unchanged in both modes: explicit 'template'.
template <typename T> struct wrap {
  template <typename U> int number_float() const { return 1; }
  template <int N> int nf() const { return N; }
};
template <typename T> int explicit_kw(const wrap<T> &w) {
  return w.template number_float<int>();
}

// Unchanged in both modes: the enclosing-scope class template still names a
// nested-name-specifier or a member template when a '>' closes the list.
template <typename T> struct B { int f() const { return 1; } };
template <typename T> struct D : B<T> {
  int g() const { return this->B<T>::f(); }
};
template <typename T> int qualified(const T &x) { return x.B<int>::f(); }
template <typename T> int member_tmpl(const wrap<T> &w) {
  return w.number_float<int>();
}

// Non-dependent object: the member is found, nothing changes.
struct plain { double number_float; };
bool non_dependent(plain a, plain b) { return a.number_float < b.number_float; }

// cl reads this as two comparisons (C4804 on bool > int).
template <typename T> struct ivalue { int number_float; };
template <typename T> bool chain(ivalue<T> x, int a, int b) {
  return x.number_float < a > b; // std-error {{expected ';' after return statement}}
}

bool use() {
  holder<int> h{};
  derived<int> d{};
  wrap<int> w;
  struct DD : B<int> {} dd;
  return h.dot(h) && h.arrow(&h) && h.paren(1, 2) &&
         h.call(1, 2) && // std-note {{in instantiation of member function 'holder<int>::call' requested here}}
         d.lt(d) && explicit_kw(w) && D<int>{}.g() && qualified(dd) &&
         member_tmpl(w) && non_dependent({}, {});
}
