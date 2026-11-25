// CL-PREDEF-WCHAR: cl 19.44 does not predefine __WCHAR_UNSIGNED__ (C or C++).
// Without the clang identity (the cl mode default) clang leaves it out too;
// the type and its limits are as before.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fno-ms-clang-identity -E -dM %s | FileCheck %s --check-prefix=HIDDEN
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fno-ms-clang-identity -x c++ -E -dM %s | FileCheck %s --check-prefix=HIDDEN
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -E -dM %s | FileCheck %s --check-prefix=IDENTITY
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /EP -- %s \
// RUN:   | FileCheck %s --check-prefix=CL
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /EP \
// RUN:   /clang:-fms-clang-identity -- %s | FileCheck %s --check-prefix=CL-IDENTITY

// HIDDEN-NOT: __WCHAR_UNSIGNED__
// HIDDEN:     #define __WCHAR_MAX__ 65535
// HIDDEN-NOT: __WCHAR_UNSIGNED__
// IDENTITY:   #define __WCHAR_UNSIGNED__ 1

#ifdef __WCHAR_UNSIGNED__
wchar_unsigned_defined
#else
wchar_unsigned_not_defined
#endif
// CL:          wchar_unsigned_not_defined
// CL-IDENTITY: wchar_unsigned_defined
