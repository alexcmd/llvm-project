// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -fsyntax-only -verify %s
void test(char *ordinary, volatile char8_t *vol) {
  __builtin_u8strlen(); // expected-error {{too few arguments}}
  __builtin_u8strlen(u8"a", u8"b"); // expected-error {{too many arguments}}
  __builtin_u8strlen(ordinary); // expected-error {{incompatible}}
  __builtin_u8strlen(vol); // expected-error {{incompatible}}
  __builtin_u8strlen(1); // expected-error {{incompatible}}
}
