// CL-DX-VOID-MAIN: cl 19.44 accepts 'main' returning void, bool, char, short,
// long or float (silent without /permissive-, C4326 level 1 with it) and
// unsigned int or const int silently; double, long long, pointers and enums
// stay errors (C3874). Microsoft compatibility matches; standard mode keeps
// the error.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=ms -DRT=void %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=ms -DRT=char %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=ms -DRT=bool %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=ms -DRT=long %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=ms -DRT=float %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=silent -DRT=unsigned %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=silent "-DRT=const int" %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=err -DRT=double %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=err "-DRT=long long" %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -fsyntax-only -verify=err "-DRT=int*" %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsyntax-only -verify=err -DRT=void %s

// RUN: %clang_cl /nologo --target=x86_64-pc-windows-msvc /W4 -DRT=void -fsyntax-only -- %s 2>&1 >/dev/null | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /permissive- -DRT=void -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=PERM %s
// PERM: warning: return type of 'main' is not 'int' [-Wmain-return-type]
// RUN: %clang_cl /nologo --target=x86_64-pc-windows-msvc /W4 /permissive- /wd4326 -DRT=void -fsyntax-only -- %s 2>&1 >/dev/null | count 0

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fms-cl-rules -emit-llvm -o - -DRT=char %s | FileCheck --check-prefix=IR %s
// IR: define dso_local noundef i8 @main(
// IR: ret i8 0

// silent-no-diagnostics
RT main(int argc, const char **argv) { // ms-warning {{return type of 'main' is not 'int'}} ms-note {{change return type to 'int'}} \
                                       // err-error {{'main' must return 'int'}}
  (void)argc; (void)argv;
}
