// CL-PATHMAP-OBJNAME: the debug prefix map (clang-cl /pathmap) also maps the
// CodeView S_OBJNAME path and the build information command line, as cl
// /pathmap does for S_OBJNAME (observed), so two build roots give the same
// object.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -gcodeview -debug-info-kind=limited \
// RUN:   -fdebug-prefix-map=/build/root=/r -fdebug-compilation-dir=/build/root \
// RUN:   -object-file-name=/build/root/sub/a.obj -emit-obj -o %t.obj %s
// RUN: llvm-readobj --codeview %t.obj | FileCheck --implicit-check-not=/build/root %s
// CHECK: ObjectName: /r/sub/a.obj
int f(void) { return 0; }
