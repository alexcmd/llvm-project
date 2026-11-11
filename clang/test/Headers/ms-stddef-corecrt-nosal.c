// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.50 -fno-ms-clang-identity -I %S/Inputs/ms-stddef-corecrt-nosal -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.50 -fno-ms-clang-identity -I %S/Inputs/ms-stddef-corecrt-nosal -x c++ -fsyntax-only -verify %s
// expected-no-diagnostics
//
// The corecrt.h-only fixture (no sal.h in the include path) covers the
// __has_include(<sal.h>) false branch of a full hosted cl stddef include:
// the compile must stay clean, corecrt.h must not be pulled in, and no SAL
// surface may appear (reviewer follow-up on ms-stddef-corecrt.c).
#include <stddef.h>
#ifdef TEST_SAL_INCLUDED
#error A corecrt-only runtime must not get SAL via stddef.h
#endif
size_t size;
