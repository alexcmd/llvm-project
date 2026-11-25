// CL-DX-BANNER: cl 19.44 prints its banner on stderr before every run
// without /nologo; our banner has cl's shape. Not with /nologo or -###.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c /Fo%t.obj -- %s 2>&1 >%t.out | FileCheck --check-prefix=ERR %s
// ERR: openrt C/C++ Compiler Version {{[0-9]+\.[0-9]+\.[0-9]+}} for x64
// RUN: FileCheck --check-prefix=OUT --input-file=%t.out %s
// OUT-NOT: Compiler Version

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t.obj -- %s 2>&1 >/dev/null | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -- %s 2>&1 | FileCheck --check-prefix=HASH %s
// HASH-NOT: Compiler Version

// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /c 2>&1 | FileCheck --check-prefix=NOSRC %s
// NOSRC: openrt C/C++ Compiler Version
// NOSRC: D8003

int f(void) { return 0; }
