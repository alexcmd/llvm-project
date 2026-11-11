// openrt UE-WE-LASTWINS (/W order): cl applies /wd and /w<l><n> for an id
// whatever the order of the /W level (cl 14.50.35717: /wd4717 /W4 and
// /w44101 /W3 are silent, both orders). The flags these render must come
// after the -Wall/-WCL4 of the /W, else the level switches the group back
// on.

// A /wd's -Wno-<group> renders after the /W flag, both orders.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /wd4717 /W4 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=ORDER %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W4 /wd4717 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=ORDER %s
// ORDER: "-WCL4"
// ORDER-SAME: "-Wno-infinite-recursion"

// Same for a level-4 /w<l> under /W3 (-Wall).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /w44101 /W3 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=ORDERL %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W3 /w44101 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=ORDERL %s
// ORDERL: "-Wall"
// ORDERL-SAME: "-Wno-unused-variable"

// Observed: /wd4717 stays off in both orders; only the unused variable
// remains (as on cl, which prints just the C4101 there).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /wd4717 /W4 -fsyntax-only -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=WD4717 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /wd4717 -fsyntax-only -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=WD4717 %s
// WD4717: warning: unused variable 'x' [-Wunused-variable]
// WD4717-NOT: warning:

// Observed: /w44101 above the final /W level is off, both orders (cl keeps
// the default C4717, here the -Winfinite-recursion of /W3).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /w44101 /W3 -fsyntax-only -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=W4L101 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W3 /w44101 -fsyntax-only -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=W4L101 %s
// W4L101: warning: all paths through this function will call itself [-Winfinite-recursion]
// W4L101-NOT: warning:

void rec(void) { rec(); }
int f(void) { int x; return 0; }
