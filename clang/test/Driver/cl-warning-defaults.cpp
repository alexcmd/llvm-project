// CL-WARNING-DEFAULTS: clang-only warnings that cl.exe does not report at the
// effective /W level are off in clang-cl, so a /W4 /WX build that is clean
// with cl.exe stays clean. They stay available by name; groups with a cl.exe
// counterpart follow that warning's level.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c -- %s 2>&1 | FileCheck --check-prefix=DEF %s
// DEF: "-Wno-parentheses" "-Wno-enum-compare"
// DEF-SAME: "-Wno-potentially-evaluated-expression"
// DEF-SAME: "-Wno-unused-variable"
// DEF-SAME: "-Wno-unused-parameter"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c -- %s 2>&1 | FileCheck --check-prefix=DEFN %s
// DEFN-NOT: "-Wshift-op-parentheses"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /W3 -- %s 2>&1 | FileCheck --check-prefix=W3 %s
// W3: "-Wall" "-Wno-parentheses"
// W3-SAME: "-Wno-potentially-evaluated-expression"
// W3-SAME: "-Wshift-op-parentheses"
// W3-SAME: "-Wno-unused-parameter"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /W3 -- %s 2>&1 | FileCheck --check-prefix=W3N %s
// W3N-NOT: "-Wno-unused-variable"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /W4 -Wno-foo -- %s 2>&1 | FileCheck --check-prefix=W4 %s
// W4: "-WCL4" "-Wno-enum-compare"
// W4-SAME: "-Wno-potentially-evaluated-expression"
// W4-SAME: "-Wno-logical-op-parentheses"
// W4-SAME: "-Wdeprecated-enum-compare"
// W4-SAME: "-Wno-foo"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /W4 -- %s 2>&1 | FileCheck --check-prefix=W4N %s
// W4N-NOT: "-Wno-parentheses"
// W4N-NOT: "-Wno-unused-parameter"
// W4N-NOT: "-Wno-sign-compare"

// Named groups and /Wall, -w, the clang driver: no defaults.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c -Wpotentially-evaluated-expression /W4 /w14062 /Zc:strictStrings -- %s 2>&1 | FileCheck --check-prefix=NAMED %s
// NAMED-NOT: "-Wno-potentially-evaluated-expression"
// NAMED-NOT: "-Wno-switch"
// NAMED-NOT: "-Wno-writable-strings"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /W4 /clang:-Wswitch -- %s 2>&1 | FileCheck --check-prefix=CLANGOPT %s
// CLANGOPT-NOT: "-Wno-switch"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /Wall -- %s 2>&1 | FileCheck --check-prefix=NONE %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /W4 /W0 -- %s 2>&1 | FileCheck --check-prefix=NONE %s
// RUN: %clang --target=x86_64-pc-windows-msvc -### -c -Wall %s 2>&1 | FileCheck --check-prefix=NONE %s
// NONE-NOT: "-Wno-potentially-evaluated-expression"

// The cl-clean code below compiles at /W4 /WX; named groups still warn.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /WX /GR -fsyntax-only -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 -Winconsistent-missing-override -Wswitch -Wunused-private-field -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=ON %s
// ON-DAG: warning: {{.*}}[-Winconsistent-missing-override]
// ON-DAG: warning: {{.*}}[-Wswitch]
// ON-DAG: warning: {{.*}}[-Wunused-private-field]
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /w14062 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=MAPPED %s
// MAPPED: warning: {{.*}}[-Wswitch]

struct B { virtual void a(); virtual void b(); virtual ~B(); };
struct D : B { void a() override; void b(); };

enum E { E1, E2, E3 };
int sw(E e) {
  switch (e) {
  case E1: return 1;
  case E2: return 2;
  }
  return 0;
}

class C {
  int used_;
  int unused_;
public:
  C() : used_(1), unused_(2) {}
  int get() const { return used_; }
};

int g(int x) {
  x = x;
  auto l = [x]() { return 1; };
  return l() + C().get();
}
