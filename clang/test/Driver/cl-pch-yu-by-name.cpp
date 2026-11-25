// CL-DX-YU-BY-NAME: cl 19.44 /Yu<h> does not open <h>: it skips the source up
// to the first #include whose name equals <h> (case-insensitive, quotes or
// angle brackets), without opening the headers before it, and loads the .pch.
// DirectX samples: util\DXSample.cpp uses the stdafx.h of its parent
// directory, which is not an include directory. "sub/stdafx.h" does not match
// (C1010). Beyond cl, another spelling of a through header that is found
// still matches.

// REQUIRES: x86-registered-target
// RUN: rm -rf %t && split-file %s %t
// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Ycstdafx.h /Fpout.pch /Fostdafx.obj -- stdafx.cpp
// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /WX /Yustdafx.h /Fpout.pch /Fouser.obj -- util/user.cpp
// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /WX /Yustdafx.h /Fpout.pch /Foupper.obj -- util/upper.cpp
// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /WX /Yustdafx.h /Fpout.pch /Foangled.obj -- util/angled.cpp
// RUN: cd %t && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /WX /Yustdafx.h /Fpout.pch /Fomissing.obj -- util/missing-before.cpp
// RUN: cd %t && not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Yustdafx.h /Fpout.pch /Fosub.obj -- util/sub.cpp 2>&1 | FileCheck --check-prefix=NOTSEEN %s
// NOTSEEN: fatal error: #include of 'stdafx.h' not seen while attempting to use precompiled header

// From the source directory, where no stdafx.h is reachable at all (the
// .pch still checks that its input file exists, a limit cl does not have).
// RUN: cd %t/util && %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /WX /Yustdafx.h /Fp../out.pch /Fouser3.obj -- user.cpp

//--- stdafx.h
#pragma once
int from_pch();
//--- stdafx.cpp
#include "stdafx.h"
//--- util/user.cpp
#include "stdafx.h"
int use() { return from_pch(); }
//--- util/upper.cpp
#include "STDAFX.H"
int use() { return from_pch(); }
//--- util/angled.cpp
#include <stdafx.h>
int use() { return from_pch(); }
//--- util/missing-before.cpp
#include "nonexistent.h"
#include "stdafx.h"
int use() { return from_pch(); }
//--- util/sub.cpp
#include "sub/stdafx.h"
int use() { return from_pch(); }
