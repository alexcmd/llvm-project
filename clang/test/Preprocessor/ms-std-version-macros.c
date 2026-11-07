// CL-STD-VERSION-MACROS: language version macros of clang-cl match cl.exe
// (observed on cl 14.44 and 14.51 with /EP).

// C
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /Tc%s 2>&1 | FileCheck -check-prefix=C-DEF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /Zc:__STDC__ /Tc%s 2>&1 | FileCheck -check-prefixes=C-DEF,STDC1 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /Za /Tc%s 2>&1 | FileCheck -check-prefixes=C-DEF,STDC1 %s
// C-DEF: V=undef
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c11 /Tc%s 2>&1 | FileCheck -check-prefixes=C11,STDC0 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c11 /Zc:__STDC__ /Tc%s 2>&1 | FileCheck -check-prefixes=C11,STDC1 %s
// C11: V=201112L
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c17 /Tc%s 2>&1 | FileCheck -check-prefixes=C17,STDC0 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c17 /Zc:__STDC__ /Tc%s 2>&1 | FileCheck -check-prefixes=C17,STDC1 %s
// C17: V=201710L
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:clatest /Tc%s 2>&1 | FileCheck -check-prefixes=CLATEST,STDC0 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:clatest /Zc:__STDC__ /Tc%s 2>&1 | FileCheck -check-prefixes=CLATEST,STDC1 %s
// CLATEST: V=202312L
// STDC0: S=undef
// STDC1: S=1

// C++: _MSVC_LANG / __cplusplus
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /Tp%s 2>&1 | FileCheck -check-prefix=CXX14 -DCPP=199711L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXX14 -DCPP=201402L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++14 /Tp%s 2>&1 | FileCheck -check-prefix=CXX14 -DCPP=199711L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++14 /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXX14 -DCPP=201402L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++14 /Zc:__cplusplus /Zc:__cplusplus- /Tp%s 2>&1 | FileCheck -check-prefix=CXX14 -DCPP=199711L %s
// CXX14: L=201402L C=[[CPP]]{{$}}
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++17 /Tp%s 2>&1 | FileCheck -check-prefix=CXX17 -DCPP=199711L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++17 /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXX17 -DCPP=201703L %s
// CXX17: L=201703L C=[[CPP]]{{$}}
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++20 /Tp%s 2>&1 | FileCheck -check-prefix=CXX20 -DCPP=199711L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++20 /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXX20 -DCPP=202002L %s
// CXX20: L=202002L C=[[CPP]]{{$}}
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++23preview /Tp%s 2>&1 | FileCheck -check-prefix=CXX23 -DCPP=199711L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++23preview /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXX23 -DCPP=202302L %s
// CXX23: L=202302L C=[[CPP]]{{$}}
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++latest /Tp%s 2>&1 | FileCheck -check-prefix=CXXLATEST -DCPP=199711L %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /EP /std:c++latest /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXXLATEST -DCPP=202400L %s
// CXXLATEST: L=202400L C=[[CPP]]{{$}}

// cc1 flags directly; without them the -std= values stay.
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-compatibility -fms-compatibility-version=19.44 -E -P -std=c17 %s | FileCheck -check-prefix=CC1-C17 %s
// CC1-C17: V=201710L
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-compatibility -fms-compatibility-version=19.44 -E -P -std=c17 -fms-no-stdc-version %s | FileCheck -check-prefix=C-DEF %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-compatibility -fms-compatibility-version=19.44 -E -P -std=c23 -fms-lang-version=202312 %s | FileCheck -check-prefix=CLATEST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-compatibility -fms-compatibility-version=19.44 -E -P -x c++ -std=c++2d -fms-lang-version=202400 %s | FileCheck -check-prefix=CXXLATEST -DCPP=202400L %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-compatibility -fms-compatibility-version=19.44 -E -P -x c++ -std=c++20 -fms-legacy-cplusplus %s | FileCheck -check-prefix=CXX20 -DCPP=199711L %s

// Plain clang for an MSVC target keeps clang's values.
// RUN: %clang --target=x86_64-pc-windows-msvc -E -P -x c -std=c17 %s | FileCheck -check-prefix=CC1-C17 %s
// RUN: %clang --target=x86_64-pc-windows-msvc -E -P -x c++ -std=c++20 %s | FileCheck -check-prefix=CXX20 -DCPP=202002L %s

#ifdef _MSVC_LANG
L=_MSVC_LANG C=__cplusplus
#endif
#ifdef __STDC_VERSION__
V=__STDC_VERSION__
#else
V=undef
#endif
#ifdef __STDC__
S=__STDC__
#else
S=undef
#endif
