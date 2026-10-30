// The fork identity appears in the clang-cl version line: openrt marker plus
// the commit, never a Microsoft product name.

// RUN: %clang_cl --version | FileCheck %s
// CHECK: clang version {{.*}} (openrt {{[0-9a-f]+}})
// CHECK-NOT: Microsoft
