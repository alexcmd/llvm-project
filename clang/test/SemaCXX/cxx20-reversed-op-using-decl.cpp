// RUN: %clang_cc1 -std=c++20 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s

// openrt UE-REVERSED-OPERATOR (r33): [over.match.oper]p4 (P2468). An
// operator== a derived class re-exposes with a using-declaration is not a
// rewrite target when a search for operator!= in the operand's class finds a
// corresponding one (here the base's). cl /std:c++20 is silent; it reports
// C2666 only without the operator!=.

namespace member_template {
template <bool bConst> struct TVariant {
  int Type = 0;
  template <bool bOther> bool operator==(const TVariant<bOther> &B) const { return Type == B.Type; }
  template <bool bOther> bool operator!=(const TVariant<bOther> &B) const { return !(*this == B); }
};
struct FVariant : TVariant<false> { using TVariant<false>::operator==; };
struct FConstVariant : TVariant<true> {
  using TVariant<true>::operator==;
  FConstVariant() = default;
  FConstVariant(const FVariant &o) { Type = o.Type; }
};
struct Path { FConstVariant GetField() const { return {}; } };
bool same(const Path &p, const FConstVariant field) { return p.GetField() == field; }
}

namespace non_template {
struct B {
  bool operator==(const B &) const;
  bool operator!=(const B &) const;
};
struct D : B { using B::operator==; };
bool f(const D d) { return D() == d; }
// operator!= re-exposed too.
struct D2 : B { using B::operator==; using B::operator!=; };
bool f2(const D2 d) { return D2() == d; }
}

namespace no_not_equal {
struct B {
  bool operator==(const B &) const; // expected-note {{ambiguity is between a regular call to this operator and a call with the argument order reversed}}
};
struct D : B { using B::operator==; };
bool f(const D d) { return D() == d; } // expected-warning {{ISO C++20 considers use of overloaded operator '==' (with operand types 'D' and 'const D') to be ambiguous despite there being a unique best viable function}}
}
