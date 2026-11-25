// CL-DX-V: cl 19.44 has no -v: D9002 "ignoring unknown option '-v'", then a
// normal compile (rc 0), or D8003 without a source file (rc 2). /clang:-v,
// -### and -fms-clang-identity keep clang's verbose output.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t.obj -v -- %s 2>&1 | FileCheck --check-prefix=V %s
// V: D9002 : skipping unrecognized option '-v'
// V-NOT: clang version
// V-NOT: "-cc1"

// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -v 2>&1 | FileCheck --check-prefix=NOSRC %s
// NOSRC: D9002 : skipping unrecognized option '-v'
// NOSRC: D8003

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t.obj /clang:-v -- %s 2>&1 | FileCheck --check-prefix=CLANGV %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t.obj -v /clang:-v -- %s 2>&1 | FileCheck --check-prefix=CLANGV %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t.obj -fms-clang-identity -v -- %s 2>&1 | FileCheck --check-prefix=CLANGV %s
// CLANGV-NOT: D9002
// CLANGV: -cc1

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c -### -- %s 2>&1 | FileCheck --check-prefix=HASH %s
// HASH: "-cc1"

int f(void) { return 0; }
