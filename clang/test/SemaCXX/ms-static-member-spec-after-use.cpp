// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only -verify=std %s

// openrt UE-STATIC-MEMBER-SPEC: cl 14.51 accepts an explicit specialization of
// a static data member after a use when the class template has no
// definition of that member; with one it reports C2908 (measured).

template <typename T> struct TVec {
  T x, y, z;
  constexpr TVec(T a, T b, T c) : x(a), y(b), z(c) {}
  static const TVec Zero;
  TVec Normal(T tol = 0, const TVec &ifZero = Zero) const { return ifZero; } // std-note {{implicit instantiation first required here}}
};
inline TVec<double> unit() { return TVec<double>(1, 0, 0).Normal(); }
template <> const TVec<double> TVec<double>::Zero(0, 0, 0); // std-error {{explicit specialization of 'Zero' after instantiation}}

template <typename T> struct V { T x; constexpr V(T a) : x(a) {} static const V One; };
inline double uc() { return V<double>::One.x; } // std-note {{implicit instantiation first required here}}
template <> const V<double> V<double>::One(1); // std-error {{explicit specialization of 'One' after instantiation}}

template <typename T> struct W { T x; constexpr W(T a) : x(a) {} static const W Two; };
template <typename T> const W<T> W<T>::Two(2);
inline double uw() { return W<double>::Two.x; } // ms-note {{implicit instantiation first required here}} std-note {{implicit instantiation first required here}}
template <> const W<double> W<double>::Two(0); // ms-error {{explicit specialization of 'Two' after instantiation}} std-error {{explicit specialization of 'Two' after instantiation}}
