// MS-PREDECLARE-MODULE: the EH/RTTI types the Microsoft compiler declares by
// itself (-fms-predeclare-eh-types, the cl mode default) come before the first
// token of the source file; a module-declaration is still the first
// declaration of the translation unit, with and without a global module
// fragment, and other leading declarations still are diagnosed.
//
// RUN: rm -rf %t && split-file %s %t
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions \
// RUN:   -fms-predeclare-eh-types -emit-module-interface %t/plain.cppm \
// RUN:   -o %t/plain.pcm -verify
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions \
// RUN:   -fms-predeclare-eh-types -emit-module-interface %t/gmf.cppm \
// RUN:   -o %t/gmf.pcm -verify
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions \
// RUN:   -fms-predeclare-eh-types -fmodule-file=plain=%t/plain.pcm \
// RUN:   -fsyntax-only %t/use.cpp -verify
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions \
// RUN:   -fms-predeclare-eh-types -fsyntax-only %t/late.cppm -verify
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions \
// RUN:   -fms-predeclare-eh-types -fsyntax-only %t/marker.cppm -verify
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions \
// RUN:   -fms-predeclare-eh-types -DDECL=int -include %t/decl.h \
// RUN:   -fsyntax-only %t/inc.cppm -verify
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions \
// RUN:   -emit-module-interface %t/off.cppm \
// RUN:   -o %t/off.pcm -verify

//--- plain.cppm
// expected-no-diagnostics
export module plain;
export int g() {
  try {
    throw 1;
  } catch (int) {
    return 2;
  }
  return 1;
}
export const _ThrowInfo *ti();

//--- gmf.cppm
// expected-no-diagnostics
module;
struct InGMF {};
export module gmf;
export int h(InGMF *) { return 0; }

//--- use.cpp
// expected-no-diagnostics
import plain;
int main() { return g() + (ti() != nullptr); }

//--- decl.h
DECL before;

//--- inc.cppm
export module inc; // expected-error {{module declaration must occur at the start of the translation unit}}
                   // expected-note@* {{add 'module;' to the start of the file to introduce a global module fragment}}

//--- off.cppm
// expected-no-diagnostics
export module off;
export int k() { return 3; }

//--- marker.cppm
#line 1 "<built-in>"
int before;
#line 4 "marker.cppm"
export module marker; // expected-error {{module declaration must occur at the start of the translation unit}}
                      // expected-note@* {{add 'module;' to the start of the file to introduce a global module fragment}}

//--- late.cppm
int before;
export module late; // expected-error {{module declaration must occur at the start of the translation unit}}
                    // expected-note@1 {{add 'module;' to the start of the file to introduce a global module fragment}}
