// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fsyntax-only -verify %s

// openrt UE-DECLSPEC-LEFT: a __declspec ahead of the class key with no
// declarator is cl's C4091 (level 1); #pragma warning(disable: 4091)
// silences it, as in cl (the UE cl branches place empty_bases there).

struct E1 {}; struct E2 {};
__declspec(empty_bases) class A : E1, E2 { int x; }; // expected-warning {{attribute 'empty_bases' is ignored, place it after "class" to apply attribute to type declaration}}
#pragma warning(push)
#pragma warning(disable : 4091)
__declspec(empty_bases) class B : E1, E2 { int x; };
#pragma warning(pop)
__declspec(empty_bases) class C : E1, E2 { int x; }; // expected-warning {{attribute 'empty_bases' is ignored}}
