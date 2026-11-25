// CL-DX-ECHO: cl 19.44 prints the file name (no directory) of each source it
// compiles, also with /nologo: on stdout, on stderr for /E, /EP and /P, and
// "Generating Code..." on stdout once more than one source was compiled (not
// with /MP). Nothing with -###.

// RUN: rm -rf %t && mkdir -p %t/sub
// RUN: cp %s %t/sub/one.c
// RUN: cp %s %t/two.c
// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c -- sub/one.c 2>/dev/null | FileCheck --check-prefix=ONE --match-full-lines %s
// ONE: one.c
// ONE-NOT: Generating

// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c -- sub/one.c two.c 2>/dev/null | FileCheck --check-prefix=TWO --match-full-lines %s
// TWO: one.c
// TWO-NEXT: two.c
// TWO-NEXT: Generating Code...

// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /MP2 -- sub/one.c two.c 2>/dev/null | FileCheck --check-prefix=MP --match-full-lines %s
// MP: one.c
// MP-NEXT: two.c
// MP-NOT: Generating

// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /E -- two.c 2>&1 >/dev/null | FileCheck --check-prefix=PP --match-full-lines %s
// PP: two.c

// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c -### -- two.c | count 0

int f(void) { return 0; }
