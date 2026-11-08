// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fsyntax-only \
// RUN:   -verify=nocompat %s

// openrt UE-NOISE: cl 14.51 ignores __declspec(dllexport/dllimport) on an
// enum silently; so does MSVC compatibility.
// expected-no-diagnostics

enum class __declspec(dllexport) E { A }; // nocompat-warning {{'dllexport' attribute only applies to functions, variables, classes, and Objective-C interfaces}}
enum __declspec(dllimport) F { B };       // nocompat-warning {{'dllimport' attribute only applies to functions, variables, classes, and Objective-C interfaces}}
