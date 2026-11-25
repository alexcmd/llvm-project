// CLANG-DIAG-SAMPLEUSE-ARGS: -fprofile-sample-use with an instrumentation PGO
// option names both options; the diagnostic was given one argument of two.

// RUN: not %clang --target=x86_64-unknown-linux -c -### -fprofile-sample-use=%s -fprofile-use=%s %s 2>&1 | FileCheck %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /c -### -fprofile-sample-use=%s -fprofile-use=%s -- %s 2>&1 | FileCheck --check-prefix=CL %s
// CHECK: error: invalid argument '-fprofile-sample-use={{.*}}' not allowed with '-fprofile-use={{.*}}'
// CL: error: invalid argument '-fprofile-sample-use={{.*}}' not allowed with '-fprofile-use={{.*}}'
