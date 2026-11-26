// CL-PATHMAP-OBJNAME: the debug prefix map (clang-cl /pathmap) also maps the
// CodeView S_OBJNAME path and the build information command line, as cl
// /pathmap does for S_OBJNAME (observed), so two build roots give the same
// object.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -gcodeview -gcodeview-command-line -debug-info-kind=limited \
// RUN:   -fdebug-prefix-map=/build/root=/r -fdebug-compilation-dir=/build/root \
// RUN:   -object-file-name=/build/root/sub/a.obj -emit-obj -o %t.obj %s
// RUN: llvm-readobj --codeview %t.obj | FileCheck --implicit-check-not=/build/root %s
// CHECK: ObjectName: /r/sub/a.obj
int f(void) { return 0; }

// Windows paths: the build command line is mapped before its backslashes are
// doubled, and only at a path boundary (C:\src2 is not C:\src).
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -gcodeview -gcodeview-command-line -debug-info-kind=limited \
// RUN:   "-fdebug-prefix-map=C:\src=X:\b" "-fdebug-compilation-dir=C:\src" \
// RUN:   "-object-file-name=C:\src\sub\a.obj" -I "C:\src2\inc" -emit-obj -o %t2.obj %s
// RUN: llvm-readobj --codeview %t2.obj | FileCheck --check-prefix=WIN %s
// WIN-DAG: "-fdebug-prefix-map=X:\\b=X:\\b"
// WIN-DAG: "-I" "C:\\src2\\inc"
// WIN-NOT: C:\\src\\
// WIN: ObjectName: X:\b\sub\a.obj

// Windows paths match without regard to case (as lld-link /pathmap), on any
// host, and a replacement is not mapped again by a later entry (no chaining).
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -gcodeview -gcodeview-command-line -debug-info-kind=limited \
// RUN:   "-fdebug-prefix-map=C:\src=X:\b" "-fdebug-prefix-map=X:\b=Y:\z" \
// RUN:   "-object-file-name=c:\SRC\sub\a.obj" -I "c:\Src\inc" -emit-obj -o %t3.obj %s
// RUN: llvm-readobj --codeview %t3.obj | FileCheck --check-prefix=CASE %s
// CASE-DAG: "-I" "X:\\b\\inc"
// CASE: ObjectName: X:\b\sub\a.obj

// Any COFF/CodeView target takes Windows paths (UEFI here).
// RUN: %clang_cc1 -triple x86_64-unknown-uefi -gcodeview -debug-info-kind=limited \
// RUN:   "-fdebug-prefix-map=C:\src=X:\b" "-object-file-name=c:\src\sub\u.obj" -emit-obj -o %t4.obj %s
// RUN: llvm-readobj --codeview %t4.obj | FileCheck --check-prefix=UEFI %s
// UEFI: ObjectName: X:\b\sub\u.obj
