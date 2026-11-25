// CL-MODULE-OPTIONS: the cl options for C++ named modules, as CMake passes
// them to a compiler it takes as MSVC (--modmapfmt=msvc). Observed with cl
// 19.44: an .ixx file, and every source with /interface or
// /internalPartition, is a module unit (also with /TP) whose built interface
// goes to /ifcOutput <file>, or is named after the module (<module>.ifc, '-'
// before a partition) in the /ifcOutput directory or the current one;
// /reference <name>=<file> names the interface to load for an import. The
// file is clang's module format; only the options follow cl.

// RUN: rm -rf %t && split-file %s %t && cd %t && mkdir out dir

// The command lines.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c -### \
// RUN:   greet.ixx /ifcOutput out/g.ifc /Foout/g.obj 2>&1 \
// RUN:   | FileCheck %s --check-prefix=IXX
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c -### \
// RUN:   /TP -interface greet.ixx -ifcOutput out/g.ifc /Foout/g.obj 2>&1 \
// RUN:   | FileCheck %s --check-prefix=IXX
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c -### \
// RUN:   /TP /interface iface.cpp /ifcOutput out/g.ifc /Foout/g.obj 2>&1 \
// RUN:   | FileCheck %s --check-prefix=IXX
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c -### \
// RUN:   /internalPartition impl.cpp /ifcOutput out/g.ifc /Foout/g.obj 2>&1 \
// RUN:   | FileCheck %s --check-prefix=IXX
// IXX-NOT:  warning:
// IXX:      "-cc1"
// IXX-SAME: "-emit-obj"
// IXX-SAME: "-fmodules-reduced-bmi"
// IXX-SAME: "-fmodule-output=out/g.ifc"
// IXX-SAME: "-o" "out/g.obj"
// IXX-NOT:  "-cc1"

// A directory, or no /ifcOutput: the frontend names the file.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c -### \
// RUN:   greet.ixx /ifcOutput dir 2>&1 | FileCheck %s --check-prefix=DIR
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c -### \
// RUN:   greet.ixx 2>&1 | FileCheck %s --check-prefix=CWD
// DIR: "-fmodule-output=dir{{/|\\\\}}"
// CWD: "-fmodule-output=.{{/|\\\\}}"

// References, with and without the name; unused without a C++ source only.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c -### \
// RUN:   /TP main.cpp /reference greet=out/g.ifc -reference greet:part=out/p.ifc \
// RUN:   /reference dir/other-part.ifc 2>&1 | FileCheck %s --check-prefix=REF
// REF-NOT:  warning:
// REF:      "-fmodule-file=greet=out/g.ifc"
// REF-SAME: "-fmodule-file=greet:part=out/p.ifc"
// REF-SAME: "-fmodule-file=other:part=dir/other-part.ifc"

// Compiles: interface partition, implementation partition, primary interface,
// implementation unit, importer.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   part.ixx /ifcOutput dir/ /Fodir/
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   /TP -internalPartition impl.cpp /ifcOutput dir /Fodir/
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   /TP -interface greet.ixx -ifcOutput out/g.ifc /Foout/g.obj \
// RUN:   -reference greet:part=dir/greet-part.ifc
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   /TP unit.cpp /Foout/unit.obj -reference greet=out/g.ifc \
// RUN:   -reference greet:part=dir/greet-part.ifc \
// RUN:   -reference greet:impl=dir/greet-impl.ifc
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   /TP main.cpp /Foout/main.obj -reference greet=out/g.ifc \
// RUN:   -reference dir/greet-part.ifc
// RUN: ls dir/greet-part.ifc dir/part.obj dir/greet-impl.ifc dir/impl.obj \
// RUN:   out/g.ifc out/g.obj out/unit.obj out/main.obj
// RUN: not ls greet.ifc greet-part.ifc greet-impl.ifc out/g.pcm dir/part.pcm

// No /ifcOutput: <module>.ifc in the current directory.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   part.ixx /Foout/p2.obj
// RUN: ls greet-part.ifc out/p2.obj

// Without its reference the import fails (exit 2, as every cl error).
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   /TP main.cpp /Foout/m2.obj 2>&1 | FileCheck %s --check-prefix=NOREF
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++20 /c \
// RUN:   /TP main.cpp /Foout/m2.obj /reference greet=out/nothere.ifc 2>&1 \
// RUN:   | FileCheck %s --check-prefix=NOFILE
// NOREF:  module 'greet' not found
// NOFILE: module file 'out/nothere.ifc' not found

//--- greet.ixx
export module greet;
export import :part;
export int answer();

//--- iface.cpp
export module greet;
export int answer();

//--- part.ixx
export module greet:part;
export int part() { return 40; }

//--- impl.cpp
module greet:impl;
int impl() { return 2; }

//--- unit.cpp
module greet;
import :impl;
int answer() { return part() + impl(); }

//--- main.cpp
import greet;
int main() { return answer() == 42 ? 0 : 1; }
