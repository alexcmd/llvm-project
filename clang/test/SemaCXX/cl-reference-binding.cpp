// CL-DX-REF-BINDING: cl 19.44 without /permissive- or /Zc:referenceBinding
// binds a non-const, non-volatile lvalue reference to a class rvalue of the
// same or a derived type (C4239, level 4), and ranks such a candidate below
// every other viable one, ellipsis included (observed: f(A&) vs f(const A&)
// -> const A&; g(A&) vs g(A&&) -> A&&; h(A&) vs h(...) -> h(...)). Not for
// non-class rvalues, user-defined conversions or volatile references.
// /permissive- and /Zc:referenceBinding turn it off, the last of them wins.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-rvalue-ref-binding -fms-cl-rules -std=c++17 -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-rvalue-ref-binding -fms-cl-rules -std=c++17 -DONLY_VALID -emit-llvm -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W4 /std:c++17 -DONLY_VALID -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=W4 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W3 /std:c++17 -DONLY_VALID -fsyntax-only -- %s 2>&1 >/dev/null | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W4 /wd4239 /std:c++17 -DONLY_VALID -fsyntax-only -- %s 2>&1 >/dev/null | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W3 /permissive- /Zc:referenceBinding- /std:c++17 -DONLY_VALID -fsyntax-only -- %s 2>&1 >/dev/null | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W3 /Zc:referenceBinding /permissive /std:c++17 -DONLY_VALID -fsyntax-only -- %s 2>&1 >/dev/null | count 0
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /W3 /permissive- /std:c++17 -DONLY_VALID -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /W3 /Zc:referenceBinding /std:c++17 -DONLY_VALID -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// W4: warning: non-const lvalue reference 'Ref &' binds a temporary of type 'Ref'; Microsoft extension [-Wmicrosoft-reference-binding]
// OFF: error:

struct A { int v; };
struct D : A {};
A make();
D maked();
A &&xv();

// Ranking: the extension is a last resort.
char f(A &);
short f(const A &);
static_assert(sizeof(f(make())) == sizeof(short), "");
char g(A &);
short g(A &&);
static_assert(sizeof(g(make())) == sizeof(short), "");
char h(A &);
short h(...);
static_assert(sizeof(h(make())) == sizeof(short), "");
char only(A &); // std-note 3 {{candidate function not viable: expects an lvalue for 1st argument}}

// Type traits and SFINAE do not see it.
static_assert(!__is_constructible(A &, A), "");
template <class T> auto sfinae(T &&t) -> decltype(only(static_cast<T &&>(t)), char());
short sfinae(...);
static_assert(sizeof(sfinae(make())) == sizeof(short), "");

struct Ref { Ref(); Ref(const Ref &); void operator=(Ref &rhs); }; // std-note {{candidate function not viable: expects an lvalue for 1st argument}}
Ref makeref();
struct V { int x; V(int a) : x(a) {} };

void use() {
  only(make());  // ms-warning {{non-const lvalue reference 'A &' binds a temporary of type 'A'}} std-error {{no matching function}}
  only(maked()); // ms-warning {{non-const lvalue reference 'A &' binds a temporary of type 'D'}} std-error {{no matching function}}
  only(xv());    // ms-warning {{binds a temporary of type 'A'}} std-error {{no matching function}}
  A &r = make(); // ms-warning {{binds a temporary}} std-error {{non-const lvalue reference to type 'A' cannot bind to a temporary of type 'A'}}
  (void)r;
  Ref x;
  x = makeref(); // ms-warning {{non-const lvalue reference 'Ref &' binds a temporary of type 'Ref'}} std-error {{no viable overloaded '='}}
  auto &fn = [&](void) { return 1; }; // ms-warning {{binds a temporary}} std-error {{cannot bind to a temporary}}
  (void)fn;
  auto gl = [&](auto &a, auto &b) { return a.x + b.x; }; // std-note {{candidate function [with a:auto = V, b:auto = V] not viable}}
  gl(V(1), V(2)); // ms-warning 2 {{binds a temporary of type 'V'}} std-error {{no matching function for call to object}}
}

#ifndef ONLY_VALID
struct B { operator A() const; };
B makeb();
void bad() {
  int &i = 1;            // ms-error {{non-const lvalue reference to type 'int' cannot bind to a temporary of type 'int'}} std-error {{cannot bind to a temporary}}
  A &c = makeb();        // ms-error {{cannot bind to a temporary}} std-error {{cannot bind to a temporary}}
  volatile A &v = make(); // ms-error {{cannot bind to a temporary}} std-error {{cannot bind to a temporary}}
  (void)i; (void)c; (void)v;
}
#endif

// IR-LABEL: define {{.*}}@"?use@@YAXXZ"()
// IR: call {{.*}}@"?only@@YADAEAUA@@@Z"(ptr
