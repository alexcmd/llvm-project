// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s

// openrt CL-IDENTITY: cl's traits behind std::is_pointer_interconvertible_with_class
// and std::is_corresponding_member (its <type_traits> uses them once
// __clang__ is gone). Values measured with cl 14.51 on the same classes.

struct A { int a; int b; };
struct B { int x; int y; };
struct C { int x; char y; };
union U { int u; float v; };
struct V { virtual void f(); int a; };
struct M { int a; private: int b; public: int get(); };
struct E {};
struct D : E { int d; int e; };
struct S1 { int a; long long b; };
struct S2 { int c; long long d; };
struct BF1 { int a; int b : 3; };
struct BF2 { int a; int b : 3; };
struct BF3 { int a; int b : 4; };

static_assert(__is_pointer_interconvertible_with_class(A, &A::a));
static_assert(!__is_pointer_interconvertible_with_class(A, &A::b));
static_assert(__is_pointer_interconvertible_with_class(U, &U::v));
static_assert(!__is_pointer_interconvertible_with_class(V, &V::a));
static_assert(!__is_pointer_interconvertible_with_class(M, &M::a));
static_assert(__is_pointer_interconvertible_with_class(D, &D::d));
static_assert(!__is_pointer_interconvertible_with_class(A, (int A::*)nullptr));

static_assert(__is_corresponding_member(A, B, &A::a, &B::x));
static_assert(__is_corresponding_member(A, B, &A::b, &B::y));
static_assert(!__is_corresponding_member(A, B, &A::a, &B::y));
static_assert(__is_corresponding_member(A, C, &A::a, &C::x));
static_assert(!__is_corresponding_member(A, C, &A::b, &C::y));
static_assert(__is_corresponding_member(S1, S2, &S1::b, &S2::d));
static_assert(__is_corresponding_member(A, A, &A::b, &A::b));
static_assert(!__is_corresponding_member(V, V, &V::a, &V::a));
static_assert(__is_corresponding_member(U, U, &U::u, &U::u));
static_assert(__is_corresponding_member(D, D, &D::e, &D::e));
static_assert(!__is_corresponding_member(A, B, (int A::*)nullptr, &B::x));
static_assert(__is_corresponding_member(BF1, BF2, &BF1::a, &BF2::a));
static_assert(__is_corresponding_member(BF1, BF3, &BF1::a, &BF3::a));

// The library form: class types deduced from the member pointers, evaluated
// when the function is constant-evaluated.
template <class S, class Mem>
constexpr bool interconvertible(Mem S::*mp) noexcept {
  return __is_pointer_interconvertible_with_class(S, mp);
}
template <class S1T, class S2T, class M1, class M2>
constexpr bool corresponding(M1 S1T::*m1, M2 S2T::*m2) noexcept {
  return __is_corresponding_member(S1T, S2T, m1, m2);
}
static_assert(interconvertible(&A::a) && !interconvertible(&A::b));
static_assert(corresponding(&A::b, &B::y) && !corresponding(&A::b, &C::y));

// As cl: the member pointer names a member of exactly that class.
constexpr bool bad2 = __is_pointer_interconvertible_with_class(B, &A::a); // expected-error {{'__is_pointer_interconvertible_with_class' needs a pointer to a data member of 'B' (got 'int A::*')}}
constexpr bool bad3 = __is_pointer_interconvertible_with_class(int, &A::a); // expected-error {{'__is_pointer_interconvertible_with_class' needs a complete class type (got 'int')}}
constexpr bool bad4 = __is_corresponding_member(A, B, &A::a, &A::b); // expected-error {{'__is_corresponding_member' needs a pointer to a data member of 'B' (got 'int A::*')}}
