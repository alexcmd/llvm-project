// Link-time switches are not /D macros: cl.exe accepts /DEBUG:*,
// /DYNAMICDEOPT* and /DEBUGTYPE:* silently when compiling (probed 14.51.36231).
// A real /D macro still reaches cc1.

// RUN: %clang_cl /DEBUG:CTYPES /c -### -- %s 2>&1 | FileCheck %s
// RUN: %clang_cl /DYNAMICDEOPT-ALLOWICF /c -### -- %s 2>&1 | FileCheck %s
// RUN: %clang_cl /DEBUGTYPE:cv /c -### -- %s 2>&1 | FileCheck %s
// RUN: %clang_cl /DFOO=1 /DEBUG /c -### -- %s 2>&1 | FileCheck --check-prefix=FOO %s

// CHECK-NOT: EBUG
// CHECK-NOT: warning
// FOO: "-D" "FOO=1"
// FOO-NOT: EBUG
