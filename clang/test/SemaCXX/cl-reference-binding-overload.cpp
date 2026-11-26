// CL-DX-REF-BINDING, cases next to the ranking (observed with cl 19.44):
//  - f(A&) next to a deleted f(A&&): f(A()) is C2280 (the deleted one).
//  - a binding inside a converting constructor's argument ranks below the
//    ellipsis too: g(A()) with char g(B), B(A&), long g(...) picks g(...).
//  - 'return a;' of a local with only A(A&): no C4239 (the implicit move pass
//    does not take the binding, the copy pass binds the lvalue).
//  - (A&)mk() with an unrelated B::operator A&(): standard cast, silent.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -fms-rvalue-ref-binding -std=c++17 -Wmicrosoft-reference-binding -Wmicrosoft-rvalue-cast-to-reference -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -fms-rvalue-ref-binding -std=c++20 -Wmicrosoft-reference-binding -Wmicrosoft-rvalue-cast-to-reference -fsyntax-only -verify %s

struct A {};

void f(A &);           // #f1
void f(A &&) = delete; // #f2
void tf() {
  f(A()); // expected-error {{call to deleted function 'f'}}
  // expected-note@#f2 {{candidate function has been explicitly deleted}}
  // expected-note@#f1 {{candidate function}}
}

void operator+(A &, int);           // #p1
void operator+(A &&, int) = delete; // #p2
void tp() {
  A() + 1; // expected-error {{overload resolution selected deleted operator '+'}}
  // expected-note@#p2 {{candidate function has been explicitly deleted}}
  // expected-note@#p1 {{candidate function}}
}

struct X { X(A &&); };
struct Y { Y(A &&); };
void amb(A &);  // a last resort: not among the ambiguous candidates
void amb(X);    // #a2
void amb(Y);    // #a3
void ta() {
  amb(A()); // expected-error {{call to 'amb' is ambiguous}}
  // expected-note@#a2 {{candidate function}}
  // expected-note@#a3 {{candidate function}}
}

struct B { B(A &); };
char g(B);
long g(...);
static_assert(sizeof(g(A())) == sizeof(long), "the ellipsis wins");

struct L { L(); L(L &); };
L ret() { L l; return l; }

struct C { operator A &(); };
C mk();
A &r = (A &)mk();
