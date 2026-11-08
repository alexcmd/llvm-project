// openrt CL-IDENTITY: -fno-ms-clang-identity (the clang-cl default) leaves out
// every macro that names clang and adds the cl 14.51 feature macros whose
// branches need nothing cl-only; cc1 keeps the clang macros by default.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -E -dM -fno-ms-clang-identity %s | FileCheck --check-prefix=NOID \
// RUN:   --implicit-check-not=__clang --implicit-check-not=__llvm__ \
// RUN:   --implicit-check-not=__VERSION__ --implicit-check-not=__GNUC \
// RUN:   --implicit-check-not=_IS_ASSIGNABLE_NOCHECK_SUPPORTED %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++14 -E -dM -fno-ms-clang-identity %s | FileCheck --check-prefix=NOID14 \
// RUN:   --implicit-check-not=_BUILTIN_LAUNDER_SUPPORTED \
// RUN:   --implicit-check-not=_NOEXCEPT_TYPES_SUPPORTED %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -E -dM %s | FileCheck --check-prefix=ID \
// RUN:   --implicit-check-not=_CONSTEXPR_CHAR_TRAITS_SUPPORTED %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -x c -E -dM -fno-ms-clang-identity %s | FileCheck --check-prefix=NOIDC \
// RUN:   --implicit-check-not=__clang --implicit-check-not=_SUPPORTED %s

// NOID-DAG: #define _BUILTIN_LAUNDER_SUPPORTED 1
// NOID-DAG: #define _CONSTEXPR_CHAR_TRAITS_SUPPORTED 1
// NOID-DAG: #define _NOEXCEPT_TYPES_SUPPORTED 1
// NOID-DAG: #define _MSC_EXTENSIONS 1

// NOID14: #define _CONSTEXPR_CHAR_TRAITS_SUPPORTED 1

// ID-DAG: #define __clang__ 1
// ID-DAG: #define __clang_major__
// ID-DAG: #define __clang_minor__
// ID-DAG: #define __clang_patchlevel__
// ID-DAG: #define __clang_version__
// ID-DAG: #define __llvm__ 1
// ID-DAG: #define __VERSION__
// ID-DAG: #define __clang_literal_encoding__
// ID-DAG: #define __clang_wide_literal_encoding__

// NOIDC: #define _MSC_EXTENSIONS 1
