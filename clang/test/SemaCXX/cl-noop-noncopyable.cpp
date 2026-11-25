// CL-DX-NOOP: cl parses the arguments of __noop and does nothing else with them:
// no conversion, no copy, no evaluation. WIL's __WI_ANALYSIS_ASSUME(m_mutex)
// expands to __noop(m_mutex) in cl mode (no __clang__), with a non-copyable
// member; clang passed the argument through "..." and called the deleted copy
// constructor.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -DVERIFY -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /std:c++17 -fsyntax-only -- %s 2>&1 | count 0

struct U { U(); U(const U &) = delete; explicit operator bool() const; };
struct H {
  U m;
  int get() { return (__noop(m), 0) ? 1 : 2; }
  int many() { return __noop(m, U(), this); }
};
int f(int);
int fn() { return __noop(f, &f, "s"); }

#ifdef VERIFY
void bad() {
  __noop(undeclared); // expected-error {{use of undeclared identifier 'undeclared'}}
}
#endif

static_assert(__noop(U()) == 0, "");
constexpr int k = __noop;
static_assert(k == 0, "");

int side = 0;
int effects() { return __noop(++side, side = 5); }

// CHECK-LABEL: define {{.*}}@"?effects@@YAHXZ"()
// CHECK-NOT: store
// CHECK: ret i32 0
// CHECK-NOT: ??0U@@
