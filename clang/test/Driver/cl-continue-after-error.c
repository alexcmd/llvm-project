// CL-MULTI-SOURCE-CONTINUE: cl 19.44 compiles every source on the command
// line after one fails (observed: cl /c bad.cpp ok1.cpp ok2.cpp writes
// ok1.obj and ok2.obj, exit 2) and links nothing.

// RUN: rm -rf %t && split-file %s %t
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /Zs -- %t/a.c %t/b.c 2>&1 | FileCheck %s
// CHECK: a.c{{.*}}error: use of undeclared identifier 'x'
// CHECK: b.c{{.*}}error: use of undeclared identifier 'y'

// RUN: mkdir %t/out
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t/out/ -- %t/a.c %t/ok.c
// RUN: ls %t/out/ok.obj
// RUN: not ls %t/out/a.obj

//--- a.c
int a(void) { return x; }

//--- b.c
int b(void) { return y; }

//--- ok.c
int ok(void) { return 0; }
