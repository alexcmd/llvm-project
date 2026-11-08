// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s

// openrt UE-NO-SPECIALIZATIONS: specializing a no_specializations template is
// cl's C5278 (level 1); #pragma warning(disable: 5278) reaches it, as the
// STL's cl branches use around their own specializations.

namespace std {
template <class T> struct [[clang::no_specializations]] rc { using type = T; }; // expected-note {{marked 'clang::no_specializations' here}}
#pragma warning(push)
#pragma warning(disable : 5278)
template <class T> struct rc<const T> { using type = T; };
#pragma warning(pop)
template <class T> struct rc<volatile T> { using type = T; }; // expected-error {{'rc' cannot be specialized}}
}
