// CL-STD-VERSION-MACROS: clang-cl passes the cc1 flags that give the
// language version macros cl.exe's values; plain clang passes none.

// C: no /std:c11|c17|clatest -> no __STDC_VERSION__.
// RUN: %clang_cl -### -c /Tc%s 2>&1 | FileCheck -check-prefix=C-NONE %s
// RUN: %clang_cl -### -c /std:c++17 /Tc%s 2>&1 | FileCheck -check-prefix=C-NONE %s
// C-NONE: "-fms-no-stdc-version"
// C-NONE-NOT: "-fms-lang-version=
// C-NONE-NOT: "-fms-legacy-cplusplus"

// RUN: %clang_cl -### -c /std:c11 /Tc%s 2>&1 | FileCheck -check-prefix=C-STD %s
// RUN: %clang_cl -### -c /std:c17 /Tc%s 2>&1 | FileCheck -check-prefix=C-STD %s
// RUN: %clang_cl -### -c /std:clatest /clang:-std=c23 /Tc%s 2>&1 | FileCheck -check-prefix=C-STD %s
// RUN: %clang_cl -### -c /clang:-std=c99 /Tc%s 2>&1 | FileCheck -check-prefix=C-STD %s
// C-STD-NOT: "-fms-no-stdc-version"
// C-STD-NOT: "-fms-lang-version=

// RUN: %clang_cl -### -c /std:clatest /Tc%s 2>&1 | FileCheck -check-prefix=C-LATEST %s
// C-LATEST: "-std=c23"
// C-LATEST: "-fms-lang-version=202312"
// C-LATEST-NOT: "-fms-no-stdc-version"

// /Za in C defines __STDC__ like /Zc:__STDC__; /Zc:__cplusplus is claimed.
// RUN: %clang_cl -### -c /Za /Zc:__cplusplus /Tc%s 2>&1 | FileCheck -check-prefix=C-ZA %s
// C-ZA-NOT: argument unused
// C-ZA: "-fms-define-stdc"

// C++: __cplusplus 199711L unless /Zc:__cplusplus (last one wins).
// RUN: %clang_cl -### -c /Tp%s 2>&1 | FileCheck -check-prefix=CXX-LEGACY %s
// RUN: %clang_cl -### -c /std:c++20 /Tp%s 2>&1 | FileCheck -check-prefix=CXX-LEGACY %s
// RUN: %clang_cl -### -c /Zc:__cplusplus /Zc:__cplusplus- /Tp%s 2>&1 | FileCheck -check-prefix=CXX-LEGACY %s
// CXX-LEGACY-NOT: argument unused
// CXX-LEGACY-NOT: "-fms-lang-version=
// CXX-LEGACY: "-fms-legacy-cplusplus"
// CXX-LEGACY-NOT: "-fms-no-stdc-version"

// RUN: %clang_cl -### -c /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXX-ZC %s
// RUN: %clang_cl -### -c /Zc:__cplusplus- /Zc:__cplusplus /Tp%s 2>&1 | FileCheck -check-prefix=CXX-ZC %s
// CXX-ZC-NOT: argument unused
// CXX-ZC-NOT: "-fms-legacy-cplusplus"

// RUN: %clang_cl -### -c /std:c++latest /Tp%s 2>&1 | FileCheck -check-prefix=CXX-LATEST %s
// CXX-LATEST: "-fms-lang-version=202400"
// CXX-LATEST: "-fms-legacy-cplusplus"
// RUN: %clang_cl -### -c /std:c++latest /clang:-std=c++2c /Tp%s 2>&1 | FileCheck -check-prefix=CXX-LATEST-STD %s
// CXX-LATEST-STD-NOT: "-fms-lang-version=

// Plain clang (also for an MSVC target) is unchanged.
// RUN: %clang -### -c --target=x86_64-pc-windows-msvc -x c %s 2>&1 | FileCheck -check-prefix=PLAIN %s
// RUN: %clang -### -c --target=x86_64-pc-windows-msvc -x c++ %s 2>&1 | FileCheck -check-prefix=PLAIN %s
// PLAIN-NOT: "-fms-no-stdc-version"
// PLAIN-NOT: "-fms-lang-version=
// PLAIN-NOT: "-fms-legacy-cplusplus"
