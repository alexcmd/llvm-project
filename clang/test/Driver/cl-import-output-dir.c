// cl writes #import's .tlh/.tli next to the object file.
// RUN: %clang_cl -### /c /Fo%t.dir/sub/x.obj -- %s 2>&1 | FileCheck %s
// CHECK: "-fms-import-output-dir={{.*}}sub"
// RUN: %clang_cl -### /c -- %s 2>&1 | FileCheck %s --check-prefix=NODIR
// NODIR-NOT: -fms-import-output-dir
