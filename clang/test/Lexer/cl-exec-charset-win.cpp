// CL-EXEC-CHARSET: the multi-byte code pages cl accepts go through the
// Windows API (a Windows host only); GB18030 (54936) and the other pages that
// take no conversion flags work too (review C3), plain ASCII is not touched.
// REQUIRES: system-windows

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:.54936 /Fo%t.54936.obj -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:.932 /Fo%t.932.obj -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:.936 /Fo%t.936.obj -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:.949 /Fo%t.949.obj -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:.950 /Fo%t.950.obj -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:shift_jis /Fo%t.sjis.obj -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:iso-8859-5 /Fo%t.8859.obj -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:us-ascii /Fo%t.ascii.obj -- %s 2>&1 | FileCheck --check-prefix=ASCII %s
// ASCII: warning: a character in this literal cannot be represented
// GB18030: U+00E9 is A8 A6; "a" stays "a".
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-cl-rules -fexec-charset .54936 -emit-llvm -o - %s | FileCheck --check-prefix=GB %s
// GB: @s = {{.*}}c"a\A8\A6\00"

extern "C" const char s[] = "aé";
