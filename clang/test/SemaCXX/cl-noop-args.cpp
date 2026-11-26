// CL-DX-NOOP: cl 19.44 (observed) rejects a braced list as an argument of
// __noop (C3321: __noop({1, 2}), __noop({})) and accepts a void call, an
// array, a function, a lambda, a dependent argument and a pack expansion;
// the arguments are not evaluated.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -DVALID -emit-llvm -o - %s | FileCheck %s

void vf();
struct U { U(); U(const U &) = delete; };

template <class T> int dep(T &t) { return __noop(t, [&] { return t; }); }
template <class... T> int pack(T... t) { return __noop(t...); }
template <class T> int deplist(T t) {
#ifndef VALID
  return __noop({t}); // expected-error {{an initializer list cannot be an argument of '__noop'}}
#else
  return 0;
#endif
}

// CHECK-LABEL: define {{.*}}@"?h@@YAHXZ"
// CHECK-NOT: call {{.*}}@"?vf@@YAXXZ"
// CHECK: ret i32
int h() {
  U u;
  int x = 1, arr[2];
  __noop(vf());
  __noop(arr, h, &h, sizeof(__noop(1)));
  int r = dep(x) + pack(1, 2.0) + pack() + __noop(u);
#ifndef VALID
  __noop({1, 2}); // expected-error {{an initializer list cannot be an argument of '__noop'}}
  __noop({});     // expected-error {{an initializer list cannot be an argument of '__noop'}}
  r += deplist(1); // expected-note {{in instantiation of function template specialization 'deplist<int>' requested here}}
#endif
  return r;
}
