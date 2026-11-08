// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fsyntax-only \
// RUN:   -verify=nocompat %s

// openrt UE-NOISE (r32): cl 14.51 ignores __declspec(dllexport/dllimport) on
// an alias-declaration or a typedef silently (/Wall); so does MSVC
// compatibility. The class itself is not exported.
// expected-no-diagnostics

template <typename T> struct THighFreq { T v; };
enum class ETex { A, B };
using FFaceData = __declspec(dllexport) THighFreq<ETex>; // nocompat-warning {{'dllexport' attribute only applies to functions, variables, classes, and Objective-C interfaces}}
using FInt = __declspec(dllimport) int;                  // nocompat-warning {{'dllimport' attribute only applies to functions, variables, classes, and Objective-C interfaces}}
typedef __declspec(dllexport) THighFreq<int> FTd;        // nocompat-warning {{'dllexport' attribute only applies to functions, variables, classes, and Objective-C interfaces}}
FFaceData g;
