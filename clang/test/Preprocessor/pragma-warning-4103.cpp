// RUN: rm -rf %t && mkdir -p %t
// RUN: echo "#pragma pack(push, 4)" > %t/push.h
// RUN: echo "#pragma pack(pop)" > %t/pop.h
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fsyntax-only -I%t -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fsyntax-only -I%t -verify=on -DON %s

// openrt UE-PRAGMA-PACK-C4103: #pragma warning(disable: 4103) silences the
// pack change across an #include (cl C4103).
// expected-no-diagnostics

#ifndef ON
#pragma warning(disable: 4103)
#endif
#include "push.h" // on-warning {{modified in the included file}} on-note@push.h:1 {{previous '#pragma pack' directive}}
struct S { char c; double d; };
#include "pop.h" // on-warning {{modified in the included file}} on-note@* {{previous '#pragma pack' directive}}
