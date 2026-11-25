// CL-DX-GL-GM: every MSBuild Release project passes /GL and /Gm-; cl 19.44 is
// silent on /GL, /GL- and /Gm- and warns D9035 for /Gm (deprecated). /GL is
// accepted without LTO (option A): the compile job has no -flto.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /O2 /GL /GL- /Gm- -### -- %s 2>&1 | FileCheck %s
// CHECK-NOT: argument unused
// CHECK-NOT: D9035
// CHECK: "-cc1"
// CHECK-NOT: "-flto{{(=[a-z]+)?}}"
// CHECK-NOT: argument unused

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Gm- /Gm -### -- %s 2>&1 | FileCheck --check-prefix=GM %s
// GM: warning: D9035 : option 'Gm' is deprecated and may be dropped in a later release
// GM-NOT: argument unused
