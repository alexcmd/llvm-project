// MSVC warning numbers map to clang groups (table in CLWarnings.cpp):
// /wd silences, /we makes an error, /wo and /w1 enable, /w0 silences.
// Unmapped ids (/wd9999) stay silent, as in cl.exe.

// RUN: %clang_cl /W4 /wd4101 -fsyntax-only -- %s 2>&1 | FileCheck --allow-empty --check-prefix=QUIET %s
// RUN: %clang_cl /W4 /w04101 -fsyntax-only -- %s 2>&1 | FileCheck --allow-empty --check-prefix=QUIET %s
// RUN: not %clang_cl /W4 /we4101 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=ERR %s
// RUN: %clang_cl /W4 /wo4101 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=WARN %s
// RUN: %clang_cl /W4 /w14101 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=WARN %s
// RUN: %clang_cl /W4 /wd9999 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=WARN %s

void f(void) { int u; }

// QUIET-NOT: unused variable
// ERR: error: unused variable 'u' [-Werror,-Wunused-variable]
// WARN: warning: unused variable 'u' [-Wunused-variable]
