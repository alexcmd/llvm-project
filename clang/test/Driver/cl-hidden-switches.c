// cl.exe 14.51 (probes_cl_decisions.md section 1). Unknown /d1 and /d2 names are
// C1007 (exit codes 2 and 1 in cl; clang-cl exits 1: GAP); /d2foo is silent in cl.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /c /d1Zi /d2foo /d2Zi /d1reportSingleClassLayoutfoo -- %s 2>&1 | FileCheck --check-prefix=SILENT %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /d1foo -- %s 2>&1 | FileCheck --check-prefix=C1007-C1XX %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /d2ZO- -- %s 2>&1 | FileCheck --check-prefix=C1007-P2 %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /d1 -- %s 2>&1 | FileCheck --check-prefix=D8004-D1 %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /w1 -- %s 2>&1 | FileCheck --check-prefix=D8004-W1 %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /w1wtest.cpp -- %s 2>&1 | FileCheck --check-prefix=D8021-W1 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /F1000 /KERNEL -- %s 2>&1 | FileCheck --check-prefix=LINK %s

// SILENT-NOT: error:
// SILENT-NOT: warning:
// C1007-C1XX: fatal error C1007: unrecognized flag '-foo' in 'c1xx'
// C1007-P2: fatal error C1007: unrecognized flag '-ZO-' in 'p2'
// D8004-D1: D8004 : '/d1' requires an argument
// D8004-W1: D8004 : '/w1' requires an argument
// D8021-W1: D8021 : invalid numeric argument '/w1wtest.cpp'
// LINK: "/STACK:0x1000"
// LINK-SAME: "/KERNEL"
