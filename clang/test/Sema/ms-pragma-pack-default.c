// RUN: rm -rf %t && mkdir -p %t
// RUN: printf '#pragma pack(push)\n#pragma pack(16)\n' > %t/pre16.h
// RUN: printf '#pragma pack(push)\n#pragma pack(8)\n' > %t/pre8.h
// RUN: printf '#pragma pack(pop)\n' > %t/post.h
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -I%t -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -I%t -verify=eight -DEIGHT %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fpack-struct=8 -fsyntax-only -I%t -verify -DEIGHT %s

// openrt UE-PRAGMA-PACK-DEFAULT: cl's C4103 compares the effective packing;
// pack(16) on x64 (pack(n) equal to /Zp) is the default, so no warning.
// expected-no-diagnostics

#ifdef EIGHT
#include "pre8.h" // eight-warning {{modified in the included file}} eight-note@pre8.h:2 {{previous '#pragma pack' directive}}
#else
#include "pre16.h"
#endif
struct S { char c; double d; };
#include "post.h" // eight-warning {{modified in the included file}} eight-note@* {{previous '#pragma pack' directive}}
