// An unknown /option is ignored with warning D9002 and compilation goes on,
// as cl.exe does (probed 14.51.36231: exit 0). It is not a missing file.

// RUN: %clang_cl /c /bogusopt %s -### 2>&1 | FileCheck %s
// RUN: %clang_cl /c /ACf %s -### 2>&1 | FileCheck --check-prefix=ACF %s

// CHECK: warning: D9002 : ignoring unknown option '/bogusopt'
// CHECK-NOT: no such file or directory
// CHECK: "-cc1"{{.*}}cl-unknown-option.c

// ACF: warning: D9002 : ignoring unknown option '/ACf'
// ACF-NOT: no such file or directory
