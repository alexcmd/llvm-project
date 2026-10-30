// cl.exe gives one JMC flag per file of the include closure, headers in
// post-order and the main file last; the empty nested header counts too
// (probes_cl_decisions.md section 2). The list is the module's
// llvm.msvc.jmc.files, read by the JMC instrumenter.
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -debug-info-kind=constructor -fjmc -I %S/Inputs -emit-llvm -o - %s | FileCheck %s

#include "jmc-nest-a.h"

int f(void) { return jmc_nest_a(); }

// CHECK: !llvm.msvc.jmc.files = !{![[B:[0-9]+]], ![[A:[0-9]+]], ![[M:[0-9]+]]}
// CHECK-DAG: ![[B]] = !DIFile(filename: "{{.*}}jmc-nest-b.h"
// CHECK-DAG: ![[A]] = !DIFile(filename: "{{.*}}jmc-nest-a.h"
// CHECK-DAG: ![[M]] = !DIFile(filename: "{{.*}}jmc-file-list.c"
