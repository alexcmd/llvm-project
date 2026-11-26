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
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fexec-charset windows-1252 -fms-source-charset=windows-1252 -emit-llvm -o - %t/bom.cpp 2>&1 | FileCheck --check-prefix=BOM %s
// BOM: bom.cpp:1:{{[0-9]+}}: warning: a character in this literal cannot be represented in the execution character set; it is written as '?' [-Wmicrosoft-unrepresentable-char]
// BOM: @n = {{.*}}c"\A9\80?\00"
// BOM: @w = {{.*}}[i16 169, i16 8364, i16 1046, i16 0]
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -emit-llvm -o - %t/nobom.cpp 2>&1 | FileCheck --check-prefix=UTF8 %s
// UTF8: @n = {{.*}}c"\C2\A9\E2\82\AC\D0\96\00"
// UTF8: @w = {{.*}}[i16 169, i16 8364, i16 1046, i16 0]

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=DEF %s
// DEF: "-fexec-charset" "windows-1252" "-fms-source-charset=windows-1252"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /utf-8 -### -- %t/nobom.cpp 2>&1 | FileCheck --check-prefix=U8 %s
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
// M65001: cp=65001

//--- nobom.cpp
extern "C" const char n[] = "©€Ж";
extern "C" const wchar_t w[] = L"©€Ж";
//--- bom.cpp
﻿extern "C" const char n[] = "©€Ж";
extern "C" const wchar_t w[] = L"©€Ж";
//--- macro.cpp
cp=_MSVC_EXECUTION_CHARACTER_SET
