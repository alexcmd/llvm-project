// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=werror -Werror=invalid-specialization %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only -verify=std %s

// openrt UE-NO-SPECIALIZATIONS: cl honors [[msvc::no_specializations]] and
// warns on a specialization (C5278, level 1; an error under /WX or /we5278),
// measured with cl 14.51. MSVC compatibility does the same; elsewhere both
// spellings keep the default error.

namespace std {
template <class T> struct [[msvc::no_specializations]] rc { using type = T; }; // ms-note 2 {{marked 'msvc::no_specializations' here}} werror-note 2 {{marked}} std-note 2 {{marked}}
template <class T> struct rc<const T> { using type = T; }; // ms-warning {{'rc' cannot be specialized}} werror-error {{'rc' cannot be specialized}} std-error {{'rc' cannot be specialized}}
template <class T> [[clang::no_specializations]] constexpr bool vb = false; // ms-note {{marked 'clang::no_specializations' here}} werror-note {{marked}} std-note {{marked}}
template <> constexpr bool vb<int> = true; // ms-warning {{'vb' cannot be specialized}} werror-error {{'vb' cannot be specialized}} std-error {{'vb' cannot be specialized}}
}
template <> struct std::rc<int> {}; // ms-warning {{'rc' cannot be specialized}} werror-error {{'rc' cannot be specialized}} std-error {{'rc' cannot be specialized}}
