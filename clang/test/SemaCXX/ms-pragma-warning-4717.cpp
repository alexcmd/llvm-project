// openrt UE-C4717: cl reports the self-recursive function at its default
// level (measured, cl 14.50.35717: C4717 fires with no /W flag and at
// /W1../W4, silent at /W0). clang-cl turns -Winfinite-recursion on from
// level 1, so the no-/W default warns like cl; an explicit disable keeps
// it off.

// cl's default level (no /W flag): restored() warns, suppressed() does not.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=DEF %s
// DEF: warning: all paths through this function will call itself [-Winfinite-recursion]
// DEF-NOT: warning:

// Explicit disables win, as in cl.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W0 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /wd4717 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -Wno-infinite-recursion -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// OFF-NOT: warning:

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -Winfinite-recursion -Werror -verify %s

volatile int counter;
#pragma warning(push)
#pragma warning(disable: 4717)
void suppressed() { ++counter; suppressed(); }
#pragma warning(pop)
void restored() { ++counter; restored(); } // expected-error {{all paths through this function will call itself}}
