// openrt UE-INTRIN-EXPLICIT-VCINC: with /X /external:I <dir with intrin.h>
// (UnrealBuildTool's VC include dir) the builtin header dir goes in right
// before that dir, so <intrin.h>/<immintrin.h> are clang's (intrinsics stay
// builtins), as with %INCLUDE%.

// RUN: rm -rf %t && mkdir -p %t/vcinc %t/other
// RUN: echo "" > %t/vcinc/intrin.h
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /X /external:I %t/other \
// RUN:   /external:I %t/vcinc -- %s 2>&1 | FileCheck %s
// CHECK: "-isystem" "{{[^"]*}}other"
// CHECK-SAME: "-isystem" "{{[^"]*}}{{/|\\\\}}include"
// CHECK-SAME: "-isystem" "{{[^"]*}}vcinc"

// Not without such a dir, nor with -nobuiltininc.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /X /external:I %t/other -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NONE %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /X /clang:-nobuiltininc \
// RUN:   /external:I %t/vcinc -- %s 2>&1 | FileCheck --check-prefix=NONE %s
// NONE: "-isystem" "{{[^"]*}}{{other|vcinc}}"
// NONE-NOT: "-isystem"
