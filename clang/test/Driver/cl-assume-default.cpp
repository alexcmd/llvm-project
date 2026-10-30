// cl.exe is silent about __assume with side effects; clang-cl defaults -Wno-assume
// in its MS mode. An explicit -Wassume still enables the warning.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -c -o %t.obj -- %s 2>&1 | FileCheck --allow-empty --check-prefix=QUIET %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -c -o %t.obj -Wassume -- %s 2>&1 | FileCheck --check-prefix=WARN %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c -o %t.obj -- %s 2>&1 | FileCheck --check-prefix=DRV %s

int f();
void g(int x) { __assume(f() > 0 && x > 0); }

// QUIET-NOT: warning
// WARN: warning: assumption is ignored because it contains (potential) side-effects [-Wassume]
// DRV: "-Wno-assume"
