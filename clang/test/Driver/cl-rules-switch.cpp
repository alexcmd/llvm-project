// CL-RULES: the cl language rules (-fms-cl-rules, on in cl mode) are separate
// from the identity macros: /clang:-fms-clang-identity brings back __clang__
// and keeps the rules (void main, (T&)rvalue, the reference binding);
// /clang:-fno-ms-cl-rules turns the rules off and keeps the identity hidden.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c -### -- %s 2>&1 | FileCheck --check-prefix=CL %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /clang:-fms-clang-identity -### -- %s 2>&1 | FileCheck --check-prefix=ID %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /clang:-fno-ms-cl-rules -### -- %s 2>&1 | FileCheck --check-prefix=NORULES %s
// RUN: %clang --target=x86_64-pc-windows-msvc -c -### -- %s 2>&1 | FileCheck --check-prefix=GNU %s
// CL: "-fno-ms-clang-identity"
// CL-SAME: "-fms-cl-rules"
// ID-NOT: "-fno-ms-clang-identity"
// ID: "-fms-cl-rules"
// NORULES: "-fno-ms-clang-identity"
// NORULES-NOT: "-fms-cl-rules"
// GNU-NOT: "-fms-cl-rules"

// With the identity back the rules still apply.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W4 /clang:-fms-clang-identity -DIDENT -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=RULES %s
// RULES: warning: non-const lvalue reference 'A &' binds a temporary of type 'A'
// Without the rules: errors as in standard C++.
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /clang:-fno-ms-cl-rules -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=STD %s
// STD: error: {{.*}}'main' must return 'int'
// STD: error: no matching function for call to 'f'

#ifdef IDENT
#ifndef __clang__
#error no __clang__ with /clang:-fms-clang-identity
#endif
#endif
struct A {};
void f(A &);
void main() { f(A()); }
