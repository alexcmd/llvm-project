// CL-FEATURE-CHECK-NAMES: to cl the feature-check names (__has_include,
// __has_cpp_attribute, ...) mean something only in #if and #elif; anywhere
// else they are identifiers that /E prints as written (cl 19.44). With the
// clang identity hidden (cl mode default) clang does the same; CLion's
// compiler probe preprocesses the three "__cidr_define_" lines below.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fno-ms-clang-identity -E -P %s | FileCheck %s --check-prefixes=TEXT,TEXT-C --match-full-lines
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fno-ms-clang-identity -x c++ -E -P %s | FileCheck %s --check-prefixes=TEXT,TEXT-CXX --match-full-lines

// With the identity (clang driver, /clang:-fms-clang-identity) nothing changes.
// RUN: not %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -x c++ -E -P %s 2>&1 | FileCheck %s --check-prefix=IDENTITY
// IDENTITY-DAG: error: '__has_include' must be used within a preprocessing directive
// IDENTITY-DAG: error: missing '(' after '__has_cpp_attribute'

// The driver: hidden in cl mode unless asked for (the CLion probe is /TP /E /w).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /E /w -- %s | FileCheck %s --check-prefix=TEXT
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TC /EP /w -- %s | FileCheck %s --check-prefix=TEXT
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /E /w /clang:-fms-clang-identity -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=IDENTITY


__cidr_define___has_include __has_include
__cidr_define___has_include_next __has_include_next
__cidr_define___has_cpp_attribute __has_cpp_attribute
// TEXT:      __cidr_define___has_include __has_include
// TEXT-NEXT: __cidr_define___has_include_next __has_include_next
// TEXT-NEXT: __cidr_define___has_cpp_attribute __has_cpp_attribute

#define ID(x) x
#define STR2(x) #x
#define STR(x) STR2(x)

// Bare, with parentheses, and through macro arguments.
a1 __has_include __has_include(<stddef.h>) ID(__has_include) STR(__has_include) ID(__has_include("x.h"))
a2 __has_include_next __has_include_next(<stddef.h>) ID(__has_include_next) STR(__has_include_next)
a3 __has_cpp_attribute __has_cpp_attribute(nodiscard) ID(__has_cpp_attribute) STR(__has_cpp_attribute)
a4 __has_c_attribute __has_c_attribute(nodiscard) ID(__has_c_attribute) STR(__has_c_attribute)
a5 __has_declspec_attribute __has_declspec_attribute(dllexport) STR(__has_declspec_attribute)
a6 __has_feature __has_feature(x) __has_extension __has_extension(x) STR(__has_feature)
a7 __has_builtin __has_builtin(x) __has_constexpr_builtin __has_constexpr_builtin(x)
a8 __has_attribute __has_attribute(x) __has_warning __has_warning("-Wall") __has_embed __has_embed("x")
a9 __is_identifier __is_identifier(x) __building_module __building_module(x)
a10 __is_target_arch __is_target_arch(x) __is_target_vendor __is_target_vendor(x) __is_target_os __is_target_os(x)
a11 __is_target_environment __is_target_environment(x) __is_target_variant_os __is_target_variant_os(x) __is_target_variant_environment __is_target_variant_environment(x)
// TEXT:      a1 __has_include __has_include(<stddef.h>) __has_include "__has_include" __has_include("x.h")
// TEXT-NEXT: a2 __has_include_next __has_include_next(<stddef.h>) __has_include_next "__has_include_next"
// TEXT-NEXT: a3 __has_cpp_attribute __has_cpp_attribute(nodiscard) __has_cpp_attribute "__has_cpp_attribute"
// TEXT-NEXT: a4 __has_c_attribute __has_c_attribute(nodiscard) __has_c_attribute "__has_c_attribute"
// TEXT-NEXT: a5 __has_declspec_attribute __has_declspec_attribute(dllexport) "__has_declspec_attribute"
// TEXT-NEXT: a6 __has_feature __has_feature(x) __has_extension __has_extension(x) "__has_feature"
// TEXT-NEXT: a7 __has_builtin __has_builtin(x) __has_constexpr_builtin __has_constexpr_builtin(x)
// TEXT-NEXT: a8 __has_attribute __has_attribute(x) __has_warning __has_warning("-Wall") __has_embed __has_embed("x")
// TEXT-NEXT: a9 __is_identifier __is_identifier(x) __building_module __building_module(x)
// TEXT-NEXT: a10 __is_target_arch __is_target_arch(x) __is_target_vendor __is_target_vendor(x) __is_target_os __is_target_os(x)
// TEXT-NEXT: a11 __is_target_environment __is_target_environment(x) __is_target_variant_os __is_target_variant_os(x) __is_target_variant_environment __is_target_variant_environment(x)

// In #if and #elif they work as before, also through a macro and as a macro
// argument; #ifdef and defined still see them.
#if __has_include(<no_such_header_at_all.h>)
b1 wrong
#elif __has_include("cl-feature-check-names.c")
b1 has_include
#endif
// TEXT: b1 has_include

#define HAS_SELF __has_include("cl-feature-check-names.c")
#if HAS_SELF && ID(__has_include("cl-feature-check-names.c"))
b2 through_macro
#endif
// TEXT: b2 through_macro

#ifdef __cplusplus
#if __has_cpp_attribute(nodiscard) && !__has_cpp_attribute(no_such_attribute)
b3 has_cpp_attribute
#endif
#else
#if __has_c_attribute(nodiscard) && !__has_c_attribute(no_such_attribute)
b3 has_c_attribute
#endif
#endif
// TEXT-CXX: b3 has_cpp_attribute
// TEXT-C:   b3 has_c_attribute

// The compiler's own headers test these in #if, so they stay as they are.
#if __has_builtin(__builtin_trap) && __has_feature(nullability) && __has_extension(nullability) && !__building_module(x) && defined(__has_include_next)
b4 clang_checks
#endif
// TEXT: b4 clang_checks

#if defined(__has_include)
b5 defined
#endif
#ifdef __has_include
b6 ifdef
#endif
// TEXT:      b5 defined
// TEXT:      b6 ifdef

// cl has none of these and nothing of ours needs them: not macros at all.
#if defined(__has_attribute) || defined(__has_warning) || defined(__has_embed) || defined(__has_constexpr_builtin) || defined(__has_declspec_attribute) || defined(__is_identifier) || defined(__is_target_arch) || defined(__is_target_vendor) || defined(__is_target_os) || defined(__is_target_environment) || defined(__is_target_variant_os) || defined(__is_target_variant_environment)
b8 wrong
#else
b8 cl_has_none
#endif
// TEXT: b8 cl_has_none
#ifdef __cplusplus
#ifndef __has_c_attribute
b9 other_language_undefined
#endif
#else
#ifndef __has_cpp_attribute
b9 other_language_undefined
#endif
#endif
// TEXT: b9 other_language_undefined

// A macro the program defines under such a name is an ordinary macro.
#undef __has_feature
#define __has_feature(x) user_##x
b7 __has_feature(y)
// TEXT: b7 user_y
