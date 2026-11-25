// CL-NO-INPUT: what cl 19.44 does without a source file.
//  - No argument at all: banner on stderr, an empty line, the usage line on
//    stdout, exit 0. The banner is our own in cl's shape ("... Version a.b.c
//    for <arch>"); the version is the one _MSC_FULL_VER reports.
//  - Only options (/nologo alone too): D8003, exit 2.
//  - An option at the end that lacks its argument: D8004 with the name after
//    a slash, exit 2.

// RUN: %clang_cl >%t.out 2>%t.err
// RUN: FileCheck %s --check-prefix=USAGE --input-file=%t.out --match-full-lines --strict-whitespace
// RUN: FileCheck %s --check-prefix=BANNER --input-file=%t.err --match-full-lines --strict-whitespace
// RUN: %clang_cl -### 2>&1 | FileCheck %s --check-prefix=NOT-BANNER
//      USAGE:usage: cl [ option... ] filename... [ /link linkoption... ]
//     BANNER:openrt C/C++ Compiler Version {{[0-9]+\.[0-9]+\.[0-9]+}} for {{x64|x86|ARM64|ARM}}
//BANNER-EMPTY:
// BANNER-NOT:{{.}}
// NOT-BANNER-NOT: Compiler Version

// RUN: not %clang_cl /nologo 2>&1 | FileCheck %s --check-prefix=D8003
// RUN: not %clang_cl /O2 /W3 2>&1 | FileCheck %s --check-prefix=D8003
// RUN: not %clang_cl /c /Fofoo.obj 2>&1 | FileCheck %s --check-prefix=D8003
// RUN: not %clang_cl /nologo /link foo.obj 2>&1 | FileCheck %s --check-prefix=D8003
// D8003: error: D8003 : missing source filename
// D8003-NOT: no input files
// D8003-NOT: usage:

// RUN: not %clang_cl /nologo /c %s /I 2>&1 | FileCheck %s --check-prefix=D8004-I
// RUN: not %clang_cl /nologo /c %s -I 2>&1 | FileCheck %s --check-prefix=D8004-I
// RUN: not %clang_cl /nologo /c %s /D 2>&1 | FileCheck %s --check-prefix=D8004-D
// RUN: not %clang_cl /nologo /c %s /FI 2>&1 | FileCheck %s --check-prefix=D8004-FI
// RUN: not %clang_cl /nologo /c %s /external:I 2>&1 | FileCheck %s --check-prefix=D8004-EXT
// RUN: not %clang_cl /nologo -sourceDependencies 2>&1 | FileCheck %s --check-prefix=D8004-SD
// RUN: not %clang_cl /nologo /ifcOutput 2>&1 | FileCheck %s --check-prefix=D8004-IFC
// D8004-I: error: D8004 : '/I' requires an argument
// D8004-D: error: D8004 : '/D' requires an argument
// D8004-FI: error: D8004 : '/FI' requires an argument
// D8004-EXT: error: D8004 : '/external:I' requires an argument
// D8004-SD: error: D8004 : '/sourceDependencies' requires an argument
// D8004-IFC: error: D8004 : '/ifcOutput' requires an argument
// D8004-I-NOT: is missing
// D8004-SD-NOT: D8003

// The clang driver keeps its own wording.
// RUN: not %clang 2>&1 | FileCheck %s --check-prefix=GNU
// RUN: not %clang -c %s -I 2>&1 | FileCheck %s --check-prefix=GNU-ARG
// GNU: error: no input files
// GNU-ARG: error: argument to '-I' is missing (expected 1 value)
