// CL-EXEC-CHARSET: cl 19.44 (observed, 1252 host) reads a source without a
// UTF-8 BOM in the ANSI code page and writes ordinary literals in it: from a
// BOM-less UTF-8 file "\xC2\xA9" stays C2 A9 and L"\xC2\xA9" is L"\u00C2\u00A9";
// from a file with a BOM "\u00A9\u20AC\u0416" is A9 80 3F with C4566 for the
// last one; /utf-8 keeps UTF-8 both ways (_MSVC_EXECUTION_CHARACTER_SET 65001,
// else 1252). The cl driver passes -fexec-charset windows-1252 and
// -fms-source-charset=windows-1252 by default.

// RUN: rm -rf %t && split-file %s %t
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fexec-charset windows-1252 -fms-source-charset=windows-1252 -emit-llvm -o - %t/nobom.cpp 2>&1 | FileCheck --check-prefix=NOBOM %s
// NOBOM-NOT: warning
// NOBOM: @n = {{.*}}c"\C2\A9\E2\82\AC\D0\96\00"
// NOBOM: @w = {{.*}}[i16 194, i16 169, i16 226, i16 8218, i16 172, i16 208, i16 8211, i16 0]
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fexec-charset windows-1252 -fms-source-charset=windows-1252 -emit-llvm -o - %t/bom.cpp 2>&1 | FileCheck --check-prefix=BOM %s
// BOM: bom.cpp:1:{{[0-9]+}}: warning: a character in this literal cannot be represented in the execution character set; it is written as '?' [-Wmicrosoft-unrepresentable-char]
// BOM: @n = {{.*}}c"\A9\80?\00"
// BOM: @w = {{.*}}[i16 169, i16 8364, i16 1046, i16 0]
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -emit-llvm -o - %t/nobom.cpp 2>&1 | FileCheck --check-prefix=UTF8 %s
// UTF8: @n = {{.*}}c"\C2\A9\E2\82\AC\D0\96\00"
// UTF8: @w = {{.*}}[i16 169, i16 8364, i16 1046, i16 0]

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=DEF %s
// DEF: "-fexec-charset" "windows-1252" "-fms-source-charset=windows-1252"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /utf-8 -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=U8 %s
// The 1252 defaults are part of the cl rules (review C6).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /clang:-fno-ms-cl-rules -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=U8 %s
// U8-NOT: "-fexec-charset"
// U8-NOT: -fms-source-charset
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:utf-8 -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=EXECU8 %s
// EXECU8-NOT: "-fexec-charset"
// EXECU8: "-fms-source-charset=windows-1252"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /source-charset:utf-8 /execution-charset:.1252 -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=SRCU8 %s
// SRCU8: "-fexec-charset" ".1252"
// SRCU8-NOT: -fms-source-charset
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:1252 -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=BAD %s
// BAD: error: C3435 : character set '1252' is not supported
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /EP -- %t/macro.cpp | FileCheck --check-prefix=M1252 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /EP /utf-8 -- %t/macro.cpp | FileCheck --check-prefix=M65001 %s
// M1252: cp=1252

// /E and /P write the execution character set, as cl: the original bytes of a
// BOM-less file, a file with a BOM converted (A9 80 3F); /utf-8 keeps UTF-8.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /E -- %t/nobom.cpp > %t/nobom.i
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /E -- %t/bom.cpp > %t/bom.i
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /E /utf-8 -- %t/bom.cpp > %t/bomu8.i
// RUN: %python -c "import sys; d = open(sys.argv[1], 'rb').read(); assert bytes.fromhex('22c2a9e282acd09622') in d" %t/nobom.i
// RUN: %python -c "import sys; d = open(sys.argv[1], 'rb').read(); assert bytes.fromhex('22a9803f22') in d" %t/bom.i
// RUN: %python -c "import sys; d = open(sys.argv[1], 'rb').read(); assert bytes.fromhex('22c2a9e282acd09622') in d" %t/bomu8.i
// M65001: cp=65001

// A realistic BOM-less file: 300 non-ASCII bytes in an early comment grow the
// buffer; locations after them, __FILE__, /E, an included header with a
// copyright sign in a comment, /sourceDependencies and /scanDependencies
// must all see the whole converted file (review C1).
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /Zs -- %t/long.cpp 2>&1 | FileCheck --check-prefix=LONG %s
// LONG: long.cpp(4,9): error: use of undeclared identifier 'undeclared_z'
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /clang:-emit-llvm /clang:-S /Fo%t/inc.ll -- %t/inc.cpp
// RUN: FileCheck --check-prefix=FILE %s < %t/inc.ll
// FILE: c"{{.*}}inc.cpp\00"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /E -- %t/long.cpp | FileCheck --check-prefix=LONGE %s
// LONGE: int z = undeclared_z;
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t/inc2.obj /sourceDependencies %t/inc.json -- %t/inc.cpp
// RUN: FileCheck --check-prefix=SRCDEP %s < %t/inc.json
// SRCDEP: hdr.h
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest /scanDependencies %t/inc.p1689 /Fo%t/inc3.obj -- %t/inc.cpp
// RUN: FileCheck --check-prefix=SCAN %s < %t/inc.p1689
// SCAN: "rules"

// Character literals are converted before the range check (cl: '\u20AC' is
// 0x80, '\u0416' is '?' with C4566); outside cl mode a character the code page
// lacks stays an error, as with iconv (upstream -fexec-charset unchanged).
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fexec-charset windows-1252 -emit-llvm -o - %t/chars.cpp 2>&1 | FileCheck --check-prefix=CHARS %s
// CHARS: warning: a character in this literal cannot be represented
// CHARS: @c1 = {{.*}}i8 -128
// CHARS: @c2 = {{.*}}i8 63
// RUN: not %clang_cc1 -triple x86_64-pc-windows-msvc -fexec-charset windows-1252 -emit-llvm -o - %t/chars.cpp 2>&1 | FileCheck --check-prefix=NOCL %s
// NOCL: error: conversion to literal encoding failed
// The single-byte code pages are built in: the same on every host.
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fexec-charset windows-1251 -emit-llvm -o - %t/cyr.cpp | FileCheck --check-prefix=CP1251 %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fexec-charset .1251 -emit-llvm -o - %t/cyr.cpp | FileCheck --check-prefix=CP1251 %s
// CP1251: @s = {{.*}}c"\C6\E8\00"

//--- nobom.cpp
extern "C" const char n[] = "ֲ©ג‚¬׀–";
extern "C" const wchar_t w[] = L"ֲ©ג‚¬׀–";
//--- bom.cpp
ן»¿extern "C" const char n[] = "ֲ©ג‚¬׀–";
extern "C" const wchar_t w[] = L"ֲ©ג‚¬׀–";
//--- macro.cpp
cp=_MSVC_EXECUTION_CHARACTER_SET
//--- long.cpp
// יייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייי
int a = 1;
int b = 2;
int z = undeclared_z;
//--- hdr.h
#pragma once
// Copyright © somebody
#define HDR_OK 1
//--- inc.cpp
#include "hdr.h"
// ייי
#if HDR_OK
extern "C" const char *f = __FILE__;
#endif
//--- chars.cpp
extern "C" const char c1 = 'ג‚¬';
extern "C" const char c2 = '׀–';
//--- cyr.cpp
extern "C" const char s[] = "׀–׀¸";
