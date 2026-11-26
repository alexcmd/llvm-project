// CL-ATTR-AFTER-TYPE: before C++17, cl 19.44 gives C5051 for [[maybe_unused]]
// after the type ("requires at least '/std:c++17'; ignored") and stays silent
// for [[nodiscard]] (observed, default /std:c++14).

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -std=c++14 -fsyntax-only -verify %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++14 -fsyntax-only -- %s 2>&1 | FileCheck %s
// CHECK: warning: attribute {{\[\[}}maybe_unused{{\]\]}} requires at least '/std:c++17'; ignored [-Wmicrosoft-attribute-requires-std]

int [[maybe_unused]] mu14; // expected-warning {{attribute [[maybe_unused]] requires at least '/std:c++17'; ignored}}
int [[nodiscard]] nd14();
void use() { nd14(); }
