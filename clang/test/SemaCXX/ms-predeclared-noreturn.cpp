// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions -std=c++20 \
// RUN:   -fsyntax-only -verify -fms-predeclare-eh-types %s
// RUN: %clang_cc1 -triple i686-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions -std=c++17 \
// RUN:   -fsyntax-only -verify -fms-predeclare-eh-types -DX86 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /Zs /std:c++latest -Xclang -verify -- %s

// openrt MS-PREDECLARED-RUNTIME-NAMES: cl 14.51 accepts a [[noreturn]]
// redeclaration or definition of its predeclared noreturn runtime functions,
// and rejects one of a predeclared function that is not noreturn (C2381).

#ifdef X86
#define SC __stdcall
#else
#define SC __cdecl
#endif

extern "C" [[noreturn]] void __cdecl __std_terminate(void) { for (;;) {} }
extern "C" [[noreturn]] void SC _CxxThrowException(void *, _ThrowInfo *);
extern "C" [[noreturn]] void __cdecl __report_rangecheckfailure(void);
extern "C" [[noreturn]] void __cdecl __report_securityfailure(unsigned long);
extern "C" [[noreturn]] void __cdecl
__report_securityfailureEx(unsigned long, unsigned long, void **);

// expected-error@+2 {{'noreturn' attribute does not appear on the first declaration}}
// expected-note@* {{previous declaration is here}}
extern "C" [[noreturn]] int __cdecl _purecall(void);

// Ordinary functions keep the rule.
void g(); // expected-note {{previous declaration is here}}
[[noreturn]] void g(); // expected-error {{'noreturn' attribute does not appear on the first declaration}}
