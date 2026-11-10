// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.50 -fno-ms-clang-identity -I %S/Inputs/ms-stddef-corecrt -D EXPECT_CRT -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.50 -fno-ms-clang-identity -I %S/Inputs/ms-stddef-corecrt -D EXPECT_CRT -x c++ -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.50 -I %S/Inputs/ms-stddef-corecrt -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-compatibility-version=19.50 -fno-ms-clang-identity -ffreestanding -I %S/Inputs/ms-stddef-corecrt -fsyntax-only -verify %s
// expected-no-diagnostics
#define __need_size_t
#include <stddef.h>
#ifdef TEST_CORECRT_INCLUDED
#error A partial stddef include must not expose corecrt
#endif
#include <stddef.h>
#if defined(EXPECT_CRT)
#ifndef TEST_CORECRT_INCLUDED
#error A full hosted cl stddef include must expose corecrt
#endif
_Check_return_ int function(void);
#elif defined(TEST_CORECRT_INCLUDED)
#error The clang personality and freestanding headers must stay independent
#endif
size_t size;
