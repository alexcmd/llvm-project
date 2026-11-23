// RUN: %clang_cc1 -std=c++14 -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -std=c++23 -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++14 -fms-extensions -fms-compatibility -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions -fms-compatibility -fdelayed-template-parsing -fsyntax-only -verify=ms %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /Zs /std:c++17 -Xclang -verify=ms -- %s

// MS-MEMBER-TEMPLATE-LOOKUP. x.name < ... with a type-dependent x: before
// C++23, [basic.lookup.classref] also looks name up in the enclosing scope, and
// a class template found there turns '<' into a template-argument list.
// Measured with cl 14.50 /permissive- (compile results only; nlohmann/json
// unit-regression1): the '<' is a less-than when no '>' closes the list, when
// the list is ill-formed (unbalanced '?'), or when an identifier or a numeric
// or character literal without ud-suffix follows the closing '>'. Any other
// follower keeps the template-id reading. An inner '<' nests only after a
// template name. Every case below compiles with cl in all /std and
// /permissive modes, except udl (cl /permissive- rejects it too). The C++20
// lambda case also tests _MSVC_LANG, so cl sees it without /Zc:__cplusplus.
// Not matched: cl /permissive accepts every shape whose comparison reading is
// valid (x.nf < a > + b, x.nf < a > (b), x.nf < a > ::c, ...); clang-cl
// follows /permissive- there and keeps the template-id reading.

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
  // Named casts nest (json.hpp: m_value.number_float op static_cast<...>(...)).
  bool cast(const holder &o) const {
    return v.number_float < static_cast<double>(o.v.number_float); // std-error {{expected '>'}} std-note {{to match this '<'}} std-error {{expected unqualified-id}}
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

// Identifier or literal after the closing '>': two comparisons for cl.
template <typename T> struct ivalue { int number_float; };
template <typename T> bool chain(ivalue<T> x, int a, int b) {
  return x.number_float < a > b; // std-error {{expected ';' after return statement}}
}
template <typename T> bool chain_lit(ivalue<T> x, int a) {
  return x.number_float < a > 1; // std-error {{expected ';' after return statement}}
}
template <typename T> bool chain_paren(ivalue<T> x, int a, int b, int c) {
  return x.number_float < a > b > (c); // std-error {{expected ';' after return statement}}
}
// Ill-formed would-be list ('?' without ':' before the '>').
template <typename T> int ternary(ivalue<T> x, int a, int b, int c) {
  return x.number_float < a ? b > (c) : 0; // std-error {{expected ':'}} std-note {{to match this '?'}} std-error {{expected expression}}
}
// Braces in the list: a ';' or '>' inside a lambda body neither ends nor
// closes it.
template <typename T> bool lambda_in(ivalue<T> x, int a, int b) {
  return x.number_float < [&] { int t = a; return t > b; }(); // std-error {{expected '>'}} std-note {{to match this '<'}} std-error {{expected unqualified-id}}
}

// Operator, '>' (also the second half of '>>') or '(' after the closing '>':
// cl keeps the template-id reading, so a static member variable template
// still gets the enclosing-scope class template's help. A less-than reading
// would fail here ("requires template arguments"). No new "use 'template'
// keyword" warning either (-verify).
template <typename T> struct svt {
  template <int N> static constexpr int number_float = N;
  template <typename U> static constexpr int typed = 2;
};
template <typename T> class typed {};
template <typename T> int var_tmpl(const svt<T> &x) {
  return x.number_float<1> + 1;
}
template <typename T> bool var_tmpl_shr(const svt<T> &x) {
  return x.number_float < 1 >> 1;
}
template <typename T> bool var_tmpl_gt(const svt<T> &x) {
  return x.typed<int> > 1;
}
template <typename T> int var_tmpl_typed(const svt<T> &x) {
  return x.typed<int> + 1;
}
template <typename T> int var_tmpl_brace(const svt<T> &x) {
  return x.number_float < int{1} > + 1;
}
#if __cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
template <typename T> int var_tmpl_lambda(const svt<T> &x) {
  return x.number_float < [] { int t = 1; return t; }() > + 1;
}
#endif

// An inner '<' nests only after a template name (lookup in the current scope
// or after '::'). Otherwise it is a less-than inside the argument list.
constexpr int N = 1, M = 2;
template <int K> constexpr int gv = K;
template <bool B> struct Q { static int g() { return B; } };
template <typename T> struct sq : Q<true> {
  template <bool B> int number_float() const { return B; }
};
template <typename T> int inner_lt(const svt<T> &x) {
  return x.number_float<1 < 2> + 1;
}
int take(int);
template <typename T> int inner_lt_call(const svt<T> &x) {
  return take(x.number_float<1 < 2>);
}
template <typename T> int inner_lt_semi(const svt<T> &x) {
  return x.number_float<1 < 2>;
}
template <typename T> bool inner_lt_shr(const svt<T> &x) {
  return x.number_float<1 < 2>> 1;
}
template <typename T> int inner_lt_const(const svt<T> &x) {
  return x.number_float<N < M> + 1;
}
template <typename T> int inner_lt_memfn(const sq<T> &x) {
  return x.number_float<1 < 2>() + x.number_float<sizeof(T) < 8>();
}
template <typename T> int inner_lt_qual(const sq<T> &x) {
  return x.Q<1 < 2>::g();
}
// 'a < b >' does not nest, so this is the template-id reading as for cl.
template <typename T> bool inner_lt_tmpl_reading(const svt<T> &x) {
  return x.number_float < N < M > + 1 && x.number_float < N < M >> 1;
}
// A template name nests: one '>' closes gv<1>, the next the list, and an
// identifier follows, so for cl these are comparisons.
template <typename T> bool inner_tmpl_nests(ivalue<T> x, int b) {
  return x.number_float < gv<1> > b; // std-error {{expected ';' after return statement}}
}
template <typename T> bool inner_tmpl_nests_qual(ivalue<T> x, int b) {
  return x.number_float < Q<true>::g() > b; // std-error {{expected ';' after return statement}}
}
// A user-defined literal after the '>' keeps the template-id reading (error
// for cl too).
int operator""_k(unsigned long long);
template <typename T> int udl(ivalue<T> x, int a) {
  return x.number_float < a > 1_k; // std-error {{expected ';' after return statement}} ms-error {{expected ';' after return statement}} std-error {{no member named 'number_float' in 'ivalue<int>'}} ms-error {{no member named 'number_float' in 'ivalue<int>'}}
}

bool use() {
  holder<int> h{};
  derived<int> d{};
  wrap<int> w;
  struct DD : B<int> {} dd;
  return h.dot(h) && h.cast(h) && h.arrow(&h) && h.paren(1, 2) &&
         h.call(1, 2) && // std-note {{in instantiation of member function 'holder<int>::call' requested here}}
         d.lt(d) && explicit_kw(w) && D<int>{}.g() && qualified(dd) &&
         member_tmpl(w) && non_dependent({}, {});
}

bool use2() {
  ivalue<int> iv{1};
  svt<int> sv;
  // chain* stay uninstantiated: clang diagnoses chained comparisons there.
  return ternary(iv, 2, 3, 4) && lambda_in(iv, 2, 3) && var_tmpl(sv) &&
         var_tmpl_shr(sv) && var_tmpl_gt(sv) && var_tmpl_typed(sv) &&
#if __cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
         var_tmpl_lambda(sv) &&
#endif
         var_tmpl_brace(sv);
}

bool use3() {
  svt<int> sv;
  sq<int> q;
  ivalue<int> iv{1};
  return inner_lt(sv) && inner_lt_call(sv) && inner_lt_semi(sv) &&
         inner_lt_shr(sv) && inner_lt_const(sv) && inner_lt_memfn(q) &&
         inner_lt_qual(q) && inner_lt_tmpl_reading(sv) &&
         udl(iv, 2); // delayed template parsing: parsed only when used. std-note {{in instantiation of function template specialization 'udl<int>' requested here}} ms-note {{in instantiation of function template specialization 'udl<int>' requested here}}
}
