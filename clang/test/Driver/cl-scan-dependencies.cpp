// CL-SCAN-DEPENDENCIES: cl /scanDependencies <file> only scans the translation
// unit: it writes the P1689 rule of the compile (CMake reads it with
// --modmapfmt=msvc), lists the headers for /showIncludes on stderr, and neither
// compiles nor links. Shapes observed with cl 19.44.

// RUN: rm -rf %t && split-file %s %t && cd %t

// A translation unit without modules: one rule, no provides, empty requires;
// primary-output is the /Fo name. Nothing is compiled.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   -showIncludes -scanDependencies plain.json -Foobj/plain.obj plain.cpp \
// RUN:   >plain.out 2>plain.err
// RUN: FileCheck %s --check-prefix=PLAIN --input-file=plain.json
// RUN: FileCheck %s --check-prefix=SHOW --input-file=plain.err
// RUN: count 0 < plain.out
// RUN: not ls obj/plain.obj plain.obj plain.exe
// PLAIN:      {
// PLAIN-NEXT:     "version": 1,
// PLAIN-NEXT:     "revision": 0,
// PLAIN-NEXT:     "rules": [
// PLAIN-NEXT:         {
// PLAIN-NEXT:             "primary-output": "obj/plain.obj",
// PLAIN-NEXT:             "outputs": [
// PLAIN-NEXT:                 "plain.json"
// PLAIN-NEXT:             ],
// PLAIN-NEXT:             "requires": []
// PLAIN-NEXT:         }
// PLAIN-NEXT:     ]
// PLAIN-NEXT: }
// PLAIN-NOT:  provides
// SHOW: Note: including file: {{.*}}h1.h

// Without /Fo the object is named after the source; - is stdout.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   /scanDependencies- /c plain.cpp | FileCheck %s --check-prefix=STDOUT
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   /scanDependencies - /c plain.cpp | FileCheck %s --check-prefix=STDOUT
// STDOUT:      "primary-output": "plain.obj",
// STDOUT-NEXT: "outputs": [
// STDOUT-NEXT:   "-"
// STDOUT-NEXT: ],
// STDOUT-NEXT: "requires": []

// An importer: the named modules are required. Parse errors after the
// directives do not matter to the scan, and /c or /E beside it is not unused.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   /scanDependencies imp.json /c /E imp.cpp 2>&1 | count 0
// RUN: FileCheck %s --check-prefix=IMP --input-file=imp.json
// IMP:      "primary-output": "imp.obj",
// IMP-NEXT: "outputs": [
// IMP-NEXT:   "imp.json"
// IMP-NEXT: ],
// IMP-NEXT: "requires": [
// IMP-NEXT:   {
// IMP-NEXT:     "logical-name": "greet"
// IMP-NEXT:   },
// IMP-NEXT:   {
// IMP-NEXT:     "logical-name": "std"
// IMP-NEXT:   }
// IMP-NEXT: ]
// IMP-NOT:  provides

// Before C++20 import is an identifier.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++17 \
// RUN:   /scanDependencies imp17.json /c imp.cpp
// RUN: FileCheck %s --check-prefix=IMP17 --input-file=imp17.json
// IMP17: "requires": []

// A module interface unit provides its module; its .ifc is an output.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest \
// RUN:   -showIncludes /scanDependencies greet.json /c /TP greet.ixx \
// RUN:   -Foobj/g.obj 2>&1 | FileCheck %s --check-prefix=SHOW
// RUN: FileCheck %s --check-prefix=IFACE --input-file=greet.json
// IFACE:      "primary-output": "obj/g.obj",
// IFACE-NEXT: "outputs": [
// IFACE-NEXT:   "greet.json",
// IFACE-NEXT:   "greet.ifc"
// IFACE-NEXT: ],
// IFACE-NEXT: "provides": [
// IFACE-NEXT:   {
// IFACE-NEXT:     "logical-name": "greet",
// IFACE-NEXT:     "source-path": "{{.*}}greet.ixx",
// IFACE-NEXT:     "is-interface": true
// IFACE-NEXT:   }
// IFACE-NEXT: ],
// IFACE-NEXT: "requires": []

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest \
// RUN:   /scanDependencies greet2.json /c /TP greet.ixx /ifcOutput out/g.ifc
// RUN: FileCheck %s --check-prefix=IFCOUT --input-file=greet2.json
// IFCOUT:      "outputs": [
// IFCOUT-NEXT:   "greet2.json",
// IFCOUT-NEXT:   "out/g.ifc"
// IFCOUT-NEXT: ],

// Partitions: an interface partition, an implementation partition (provided,
// not an interface) and an implementation unit (requires its module).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest \
// RUN:   /scanDependencies part.json /c /TP part.ixx
// RUN: FileCheck %s --check-prefix=PART --input-file=part.json
// PART:      "outputs": [
// PART-NEXT:   "part.json",
// PART-NEXT:   "greet-part.ifc"
// PART-NEXT: ],
// PART-NEXT: "provides": [
// PART-NEXT:   {
// PART-NEXT:     "logical-name": "greet:part",
// PART-NEXT:     "source-path": "{{.*}}part.ixx",
// PART-NEXT:     "is-interface": true
// PART-NEXT:   }
// PART-NEXT: ],
// PART-NEXT: "requires": [
// PART-NEXT:   {
// PART-NEXT:     "logical-name": "greet:impl"
// PART-NEXT:   }
// PART-NEXT: ]

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest \
// RUN:   /scanDependencies implpart.json /c implpart.cpp
// RUN: FileCheck %s --check-prefix=IMPLPART --input-file=implpart.json
// IMPLPART:      "logical-name": "greet:impl",
// IMPLPART-NEXT: "source-path": "{{.*}}implpart.cpp",
// IMPLPART-NEXT: "is-interface": false

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest \
// RUN:   /scanDependencies implunit.json /c implunit.cpp
// RUN: FileCheck %s --check-prefix=IMPLUNIT --input-file=implunit.json
// IMPLUNIT-NOT:  provides
// IMPLUNIT:      "requires": [
// IMPLUNIT-NEXT:   {
// IMPLUNIT-NEXT:     "logical-name": "greet"
// IMPLUNIT-NEXT:   },
// IMPLUNIT-NEXT:   {
// IMPLUNIT-NEXT:     "logical-name": "greet:impl"
// IMPLUNIT-NEXT:   }
// IMPLUNIT-NEXT: ]

// An .ixx file is scanned as C++ without /TP, also with the module options
// of the compile beside it.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /std:c++latest \
// RUN:   /scanDependencies ixx.json /c greet.ixx /interface 2>&1 | count 0
// RUN: FileCheck %s --check-prefix=IXX --input-file=ixx.json
// IXX:      "primary-output": "greet.obj",
// IXX:      "logical-name": "greet",
// IXX-NEXT: "source-path": "{{.*}}greet.ixx",
// IXX-NEXT: "is-interface": true

// /sourceDependencies beside the scan is written and is an output.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   /scanDependencies sd.json /sourceDependencies sd.deps.json /c plain.cpp
// RUN: FileCheck %s --check-prefix=SRCDEP --input-file=sd.json
// RUN: FileCheck %s --check-prefix=SRCDEP-FILE --input-file=sd.deps.json
// SRCDEP:      "outputs": [
// SRCDEP-NEXT:   "sd.json",
// SRCDEP-NEXT:   "sd.deps.json"
// SRCDEP-NEXT: ],
// SRCDEP-FILE: "Includes": [
// SRCDEP-FILE-NEXT: "{{.*}}h1.h"

// Header unit imports: the header as written, where it was found and how it
// was looked up. Nothing has to be built for the scan.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   /scanDependencies hu.json /c hu.cpp /Iinc 2>&1 | count 0
// RUN: FileCheck %s --check-prefix=HU --input-file=hu.json
// HU:      "requires": [
// HU-NEXT:   {
// HU-NEXT:     "logical-name": "h1.h",
// HU-NEXT:     "source-path": "{{.*}}h1.h",
// HU-NEXT:     "lookup-method": "include-quote",
// HU-NEXT:     "unique-on-source-path": true
// HU-NEXT:   },
// HU-NEXT:   {
// HU-NEXT:     "logical-name": "sys.h",
// HU-NEXT:     "source-path": "{{.*}}inc{{.*}}sys.h",
// HU-NEXT:     "lookup-method": "include-angle",
// HU-NEXT:     "unique-on-source-path": true
// HU-NEXT:   },
// HU-NEXT:   {
// HU-NEXT:     "logical-name": "greet"
// HU-NEXT:   }
// HU-NEXT: ]

// cl scans C++ only: for a C source it is C1007 from the C front end and no
// file.
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /TC \
// RUN:   /scanDependencies c.json /c plain.cpp 2>&1 | FileCheck %s --check-prefix=TC
// RUN: not ls c.json
// TC: c1: fatal error C1007: unrecognized flag '-scanDependenciesc.json' in 'c1'

// A header that is not found fails the scan (exit 2, as every cl error) and
// leaves the file empty.
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   /scanDependencies missinc.json /c missinc.cpp 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MISSINC
// RUN: count 0 < missinc.json
// MISSINC: nonexistent_header.h

// Without its argument the option is an error as before (D8004 for an empty
// one) and nothing is scanned.
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /c plain.cpp \
// RUN:   /scanDependencies 2>&1 | FileCheck %s --check-prefix=NOARG
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /c plain.cpp \
// RUN:   /scanDependencies "" 2>&1 | FileCheck %s --check-prefix=EMPTYARG
// NOARG: argument to '/scanDependencies' is missing
// EMPTYARG: D8004 : '/scanDependencies' requires an argument

// -### prints the compile the scan is taken from and scans nothing.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /TP /std:c++latest \
// RUN:   /scanDependencies hash.json plain.cpp -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=HASH
// RUN: not ls hash.json
// HASH:     "-cc1"
// HASH-NOT: lld-link

//--- h1.h
inline int f() { return 0; }

//--- inc/sys.h
inline int s() { return 0; }

//--- hu.cpp
import "h1.h";
import <sys.h>;
import greet;
int main() { return f() + s(); }

//--- plain.cpp
#include "h1.h"
int main() { return f(); }

//--- imp.cpp
#include "h1.h"
import greet;
import std;
int main( { return f() }

//--- greet.ixx
module;
#include "h1.h"
export module greet;
export int g() { return f(); }

//--- part.ixx
export module greet:part;
import :impl;
export int p();

//--- implpart.cpp
module greet:impl;
int q();

//--- implunit.cpp
module greet;
import :impl;
int g2() { return 1; }

//--- missinc.cpp
#include "nonexistent_header.h"
import greet;
int main() { return 0; }
