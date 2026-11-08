// openrt UE-NOISE: clang warnings cl 14.51 does not give at any /W level
// (measured on minimal repros; UnrealEditor: cl 0 warnings, openrt
// 18256 + 1772 + 1743 + 686 + 26 + 6) are off in clang-cl; the comma one is
// cl's C4548 (off by default) and comes back with /w14548.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W4 -- %s 2>&1 | FileCheck %s
// CHECK-DAG: "-Wno-implicit-exception-spec-mismatch"
// CHECK-DAG: "-Wno-extra-qualification"
// CHECK-DAG: "-Wno-unused-comma-left-operand"
// CHECK-DAG: "-Wno-microsoft-unqualified-friend"
// CHECK-DAG: "-Wno-microsoft-cast"
// CHECK-DAG: "-Wno-invalid-unevaluated-string"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W4 /w14548 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=C4548 %s
// C4548-NOT: "-Wno-unused-comma-left-operand"
// C4548: "-Wunused-comma-left-operand"
