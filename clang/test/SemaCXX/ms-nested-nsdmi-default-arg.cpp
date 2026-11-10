// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=ms -DMS %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only -verify=std %s

// openrt UE-NSDMI-DEFAULT-ARG (r34): a nested struct with default member
// initializers, default-constructed in a default argument of the enclosing
// class's member functions. cl accepts it; so does MSVC compatibility.
// ms-no-diagnostics

struct FText {};
template <class T> struct TArray { T *p = nullptr; };
struct UStaticMesh {
  struct FBuildParameters { // std-error {{default member initializer for 'bInSilent' needed within definition of enclosing class 'UStaticMesh' outside of member functions}} std-error {{default member initializer for 'OutErrors' needed}}
    bool bInSilent = false; // std-note {{default member initializer declared here}}
    TArray<FText> *OutErrors = nullptr; // std-note {{default member initializer declared here}}
  };
  void Build(const FBuildParameters &P = FBuildParameters()); // std-note {{in evaluation of exception specification}}
};
void f(UStaticMesh &m) { m.Build(); }

// The exception specification follows the initializers once they are parsed
// (cl 14.51: noexcept(N1()) is false, noexcept(N2()) true after the class).
static_assert(noexcept(UStaticMesh::FBuildParameters()));
#ifdef MS
int mayThrow();
int noThrow() noexcept;
struct O {
  struct N1 { int a = mayThrow(); };
  struct N2 { int a = noThrow(); };
  void f1(N1 n = N1());
  void f2(N2 n = N2());
};
static_assert(!noexcept(O::N1()));
static_assert(noexcept(O::N2()));

// Transitive member/base dependencies must be revisited as well. These
// containing classes have no default member initializers of their own.
struct Transitive {
  struct Throwing { int a = mayThrow(); };
  struct Member { Throwing a; };
  struct Base : Member {};
  struct Safe { int a = noThrow(); };
  struct SafeMember { Safe a; };
  void f(Base b = Base());
  void g(SafeMember b = SafeMember());
};
static_assert(!noexcept(Transitive::Throwing()));
static_assert(!noexcept(Transitive::Member()));
static_assert(!noexcept(Transitive::Base()));
static_assert(noexcept(Transitive::SafeMember()));

// The first constructor encountered can depend, through a parsed initializer,
// on a provisional constructor that was encountered later.
struct ReverseOrder {
  struct A { int a = mayThrow(); };
  struct B { A a = A(); };
  void b(B b = B());
  void a(A a = A());
};
static_assert(!noexcept(ReverseOrder::B()));

// Parsing a local class inside an initializer must not flush the enclosing
// class's pending definitions while that initializer is still unparsed.
struct NestedParse {
  struct Leaf {
    int value = [] { struct Local {}; return mayThrow(); }();
  };
  struct Member { Leaf leaf; };
  void trigger(Member value = Member());
};
static_assert(!noexcept(NestedParse::Member()));
#endif
