// clang-cl reports a failed compile through the cl.exe exit path (2). The
// status is probed by hand; lit checks the diagnostic only.
// RUN: not %clang_cl /c %s/no-such-source.c 2>&1 | FileCheck %s
// CHECK: no such file or directory
