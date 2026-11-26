// CL-STD-PERMISSIVE: cl 19.44 takes /std:c++20, /std:c++23preview and
// /std:c++latest as /permissive- at their place among /permissive,
// /permissive-, /Zc:referenceBinding[-] (observed: /std:c++20 makes f(A())
// for f(A&) C2664 and &T() C2102; /std:c++20 /permissive and /std:c++20
// /Zc:referenceBinding- bring the binding back, /permissive /std:c++20 does
// not; /std:c++17 changes nothing).

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++17 -### -- %s 2>&1 | FileCheck --check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++20 /permissive -### -- %s 2>&1 | FileCheck --check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++latest /Zc:referenceBinding- -### -- %s 2>&1 | FileCheck --check-prefix=BIND %s
// ON: "-fms-rvalue-ref-binding"
// ON: "-Wno-error=address-of-temporary"
// BIND: "-fms-rvalue-ref-binding"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++20 -### -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++latest -### -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++23preview -### -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /permissive /std:c++20 -### -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// OFF-NOT: "-fms-rvalue-ref-binding"
// OFF-NOT: "-Wno-error=address-of-temporary"

// What /permissive turns on (no two-phase lookup) follows the same order:
// /permissive /std:c++20 has two-phase lookup, /std:c++20 /permissive not.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /permissive /std:c++20 -### -- %s 2>&1 | FileCheck --check-prefix=TWOPHASE %s
// TWOPHASE-NOT: "-fdelayed-template-parsing"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++20 /permissive -### -- %s 2>&1 | FileCheck --check-prefix=DELAYED %s
// DELAYED: "-fdelayed-template-parsing"
// cl prints nothing about delayed template parsing for /std:c++20 /permissive.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c++20 /permissive -### -- %s 2>&1 | FileCheck --check-prefix=NONOTE %s
// NONOTE-NOT: delayed-template-parsing is deprecated
