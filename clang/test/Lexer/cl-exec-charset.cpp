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
// BOM: bom.cpp:1:{{[0-9]+}}: warning: a character in this literal cannot be represented in the execution character set [-Wmicrosoft-unrepresentable-char]
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

// #embed takes the bytes on disk, never converted (review N1).
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fexec-charset windows-1252 -fms-source-charset=windows-1252 -std=c23 -x c -emit-llvm -o - %t/emb.c | FileCheck --check-prefix=EMBED %s
// EMBED: @d = {{.*}}c"A\80\E9\FFB
// The CodeView file checksum is that of the file on disk (review N2).
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fms-source-charset=windows-1252 -gcodeview -debug-info-kind=limited -emit-llvm -o %t/ck.ll %t/ck.cpp
// RUN: %python %t/md5check.py %t/ck.cpp %t/ck.ll
// -frewrite-includes output starts with a UTF-8 BOM, so a compile of it gives
// the same literals as the compile of the sources (review N3).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /E /clang:-frewrite-includes -- %t/rw.cpp > %t/rw.i
// RUN: %python -c "import sys; assert open(sys.argv[1], 'rb').read(3) == bytes.fromhex('efbbbf')" %t/rw.i
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /TP /clang:-emit-llvm /clang:-S /Fo%t/rw1.ll -- %t/rw.i
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /clang:-emit-llvm /clang:-S /Fo%t/rw2.ll -- %t/rw.cpp
// RUN: FileCheck --check-prefix=RW %s < %t/rw1.ll
// RUN: FileCheck --check-prefix=RW %s < %t/rw2.ll
// RW: @rws = {{.*}}c"\E9t\E9\00"
// A main file that starts with a byte order mark (the rewriter does not copy
// it): the output still starts with one (review S1).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /E /clang:-frewrite-includes -- %t/rwb.cpp > %t/rwb.i
// RUN: %python -c "import sys; assert open(sys.argv[1], 'rb').read(3) == bytes.fromhex('efbbbf')" %t/rwb.i
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /TP /clang:-emit-llvm /clang:-S /Fo%t/rwb1.ll -- %t/rwb.i
// RUN: FileCheck --check-prefix=RWB %s < %t/rwb1.ll
// RWB: @bs = {{.*}}c"\E9\00"
// The scanner decodes the sources as the compile does: a non-ASCII include
// name in a 1252 source (review N4), also through clang-scan-deps.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Fo%t/sc.obj -- %t/sc.cpp
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest /scanDependencies %t/sc.p1689 /Fo%t/sc2.obj -- %t/sc.cpp
// RUN: clang-scan-deps -format=p1689 -- %clang_cl --target=x86_64-pc-windows-msvc /std:c++latest /c /Fo%t/sc3.obj -- %t/sc.cpp > %t/sc3.json
// A PCH is refused with another /execution-charset (cl C2855; review N5).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Ycph.h /Fp%t/ph.pch /Fo%t/ph.obj -- %t/ph.cpp
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Yuph.h /Fp%t/ph.pch /execution-charset:utf-8 /Fo%t/pu.obj -- %t/ph.cpp 2>&1 | FileCheck --check-prefix=PCHX %s
// PCHX: C2855: command-line option '/execution-charset' differs in precompiled file
// The AST content hash is that of the file on disk (review N6): a header
// with a new time but the same contents keeps the PCH usable.
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fms-source-charset=windows-1252 -fvalidate-ast-input-files-content -x c++-header -emit-pch -o %t/ch.pch %t/ch.h
// RUN: touch -m -d '2001-01-01 00:00:00' %t/ch.h
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fms-source-charset=windows-1252 -fvalidate-ast-input-files-content -include-pch %t/ch.pch -fsyntax-only %t/chu.cpp

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
//--- bin.dat
A€יÿB
//--- emb.c
const unsigned char d[] = {
#embed "bin.dat"
};
//--- ck.cpp
// cafי
int fck() { return 0; }
//--- md5check.py
import hashlib, sys
h = hashlib.md5(open(sys.argv[1], 'rb').read()).hexdigest()
ir = open(sys.argv[2]).read()
assert 'checksum: "%s"' % h in ir, h
//--- rh.h
extern "C" const char rws[] = "יtי";
//--- rw.cpp
#include "rh.h"
// cafי
//--- hֳ©.h
#define HE 1
//--- sc.cpp
#include "hי.h"
int sc = HE;
//--- ph.h
extern "C" const char phs[] = "י";
//--- ph.cpp
#include "ph.h"
//--- ch.h
// ייי
int ch_value = 1;
//--- chu.cpp
int use_ch() { return ch_value; }
//--- rwb.cpp
ן»¿extern "C" const char bs[] = "ֳ©";
