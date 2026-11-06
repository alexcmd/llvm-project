// /E output leaves the pre-injected EH/RTTI types out (they are not source
// text); compiling the preprocessed file gets them again.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /E -- %s > %t.ii
// RUN: FileCheck %s < %t.ii
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /Zs -- %t.ii
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /Zs /TP -- %t.ii
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /Zs \
// RUN:   -fno-ms-predeclare-eh-types -- %t.ii 2>&1 | FileCheck --check-prefix=OFF %s

// CHECK-NOT: _s__ThrowInfo
// CHECK: void f(_ThrowInfo *p, __RTTICompleteObjectLocator *q);
// OFF: unknown type name
void f(_ThrowInfo *p, __RTTICompleteObjectLocator *q);
