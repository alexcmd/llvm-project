// OPENRT-PREDEF: clang-cl predefines __OPENRT__ (major*100+minor of the openrt
// release, CMake CLANG_OPENRT_RELEASE); _MSC_VER is left as it is. The clang
// driver, also for an MSVC target, does not define it.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CC1
// CC1: "-D__OPENRT__={{[1-9][0-9]*}}"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /E /clang:-dM -- %s \
// RUN:   | FileCheck %s --check-prefix=CL
// CL-DAG: #define _MSC_VER {{[0-9]+}}
// CL-DAG: #define __OPENRT__ {{[1-9][0-9]*}}

// RUN: %clang --target=x86_64-pc-windows-msvc -E -dM %s \
// RUN:   | FileCheck %s --check-prefix=GNU --implicit-check-not=__OPENRT__
// GNU: #define _MSC_VER
