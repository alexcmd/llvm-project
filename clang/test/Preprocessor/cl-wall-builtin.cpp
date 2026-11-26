// CL-WALL-BUILTIN: the EH/RTTI types and runtime names the compiler
// predeclares (<built-in>) give no warning, also with /Wall (-Weverything):
// cl /Wall on an empty source is silent from the compiler's side (observed).

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /Wall /Zs -- %s 2>&1 | FileCheck --allow-empty %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /Wall /Zs /TC -- %s 2>&1 | FileCheck --allow-empty %s
// CHECK-NOT: <built-in>
// CHECK-NOT: warning:
int f(void);
