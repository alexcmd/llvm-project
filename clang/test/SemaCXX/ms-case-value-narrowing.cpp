// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only -verify=std %s

// openrt UE-CASE-NARROWING (Horde): cl 14.51 (/permissive-) accepts a case
// label that does not fit the switch condition's type: silently when only
// the sign changes (case -1 on size_t/unsigned), with a warning when bits are
// lost (C4310); duplicate values stay an error.

typedef unsigned long long size_t;
struct Import { static const int InvalidBaseIdx = -1; static const int CurrentBaseIdx = -3; };

int s1(size_t b) {
  switch (b) {
  case Import::InvalidBaseIdx: return 0; // std-error {{case value evaluates to -1, which cannot be narrowed to type 'size_t'}}
  case Import::CurrentBaseIdx: return 1; // std-error {{case value evaluates to -3, which cannot be narrowed to type 'size_t'}}
  default: return 2;
  }
}
int s2(unsigned b) { switch (b) { case -1: return 0; default: return 2; } } // std-error {{cannot be narrowed}}
int s3(int b) { switch (b) { case 0x100000000LL: return 0; default: return 2; } } // expected-warning {{overflow converting case value to switch condition type (4294967296 to 0)}} std-error {{cannot be narrowed}}
int s4(size_t b) {
  switch (b) {
  case (size_t)-1: return 0; // expected-note {{previous case}} std-note {{previous case}}
  case -1: return 1;         // expected-error {{duplicate case value}} std-error {{cannot be narrowed}} std-error {{duplicate case value}}
  default: return 2;
  }
}
