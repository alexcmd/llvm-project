// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -Winfinite-recursion -Werror -verify %s

volatile int counter;
#pragma warning(push)
#pragma warning(disable: 4717)
void suppressed() { ++counter; suppressed(); }
#pragma warning(pop)
void restored() { ++counter; restored(); } // expected-error {{all paths through this function will call itself}}
