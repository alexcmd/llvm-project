// CL-EXEC-CHARSET: two TUs read the same header (a non-ASCII byte, no BOM)
// in different source character sets (cl default 1252, and /utf-8): each
// gets directives scanned from its own conversion, in either order
// (review B1: the shared directive cache was keyed by file only).
//
// RUN: rm -rf %t && split-file %s %t
// RUN: sed -e "s|DIR|%/t|g" %t/ab.json.in > %t/ab.json
// RUN: sed -e "s|DIR|%/t|g" %t/ba.json.in > %t/ba.json
// RUN: clang-scan-deps -compilation-database %t/ab.json -j 1 | FileCheck %s
// RUN: clang-scan-deps -compilation-database %t/ba.json -j 1 | FileCheck %s
// CHECK-DAG: a.obj:
// CHECK-DAG: b.obj:
// CHECK-DAG: x.h
// CHECK-DAG: y.h
// RUN: clang-scan-deps -compilation-database %t/ab.json -j 1 | FileCheck --check-prefix=TWICE %s
// TWICE: x.h
// TWICE: x.h

//--- h.h
// יייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייייי
#include "x.h"
#include "y.h"
//--- x.h
int x;
//--- y.h
int y;
//--- a.cpp
#include "h.h"
//--- b.cpp
#include "h.h"
//--- ab.json.in
[
  {"directory": "DIR", "command": "clang-cl /c /FoDIR/a.obj -- DIR/a.cpp", "file": "DIR/a.cpp"},
  {"directory": "DIR", "command": "clang-cl /c /utf-8 /FoDIR/b.obj -- DIR/b.cpp", "file": "DIR/b.cpp"}
]
//--- ba.json.in
[
  {"directory": "DIR", "command": "clang-cl /c /utf-8 /FoDIR/b.obj -- DIR/b.cpp", "file": "DIR/b.cpp"},
  {"directory": "DIR", "command": "clang-cl /c /FoDIR/a.obj -- DIR/a.cpp", "file": "DIR/a.cpp"}
]
