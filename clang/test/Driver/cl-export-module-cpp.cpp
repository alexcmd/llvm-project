// CL-DX-EXPORT-MODULE: cl 19.44 compiles a .cpp as a module interface only with
// /interface (an .ixx always is one); "export module m;" in a plain .cpp is
// C3378. The clang driver keeps treating it as an interface.

// RUN: rm -rf %t && mkdir -p %t
// RUN: cp %s %t/m.cpp
// RUN: cp %s %t/m.ixx
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c /Fo%t/a.obj -- %t/m.cpp 2>&1 | FileCheck --check-prefix=ERR %s
// ERR: error: a module interface unit needs /interface or an .ixx source; a declaration can be exported only from a module interface unit
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c /interface /Fo%t/b.obj -- %t/m.cpp
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c /Fo%t/c.obj -- %t/m.ixx
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c /clang:-fms-clang-identity /Fo%t/d.obj -- %t/m.cpp
// RUN: %clang --target=x86_64-pc-windows-msvc -std=c++20 -c -o %t/e.obj %t/m.cpp

export module m;
export int f() { return 1; }
