// CL-MODULE-OPTIONS: a scan reports 'import "h.h";' and 'import <w.h>;' as
// header unit requirements (P1689R5 lookup-method, unique-on-source-path),
// never as named modules: -format=p1689 gives the header unit fields and
// -format=experimental-full keeps them out of named-module-deps.
// UNSUPPORTED: target={{.*}}-aix{{.*}}
//
// RUN: rm -rf %t && split-file %s %t
// RUN: clang-scan-deps -format=p1689 \
// RUN:   -- %clang++ -std=c++20 -I%t/inc -c %t/a.cppm -o %t/a.o \
// RUN:   | sed 's:\\\\\?:/:g' | FileCheck %s --check-prefix=P1689 -DPREFIX=%/t
// RUN: clang-scan-deps -format=experimental-full \
// RUN:   -- %clang++ -std=c++20 -I%t/inc -c %t/a.cppm -o %t/a.o \
// RUN:   | FileCheck %s --check-prefix=FULL

// P1689:      "provides": [
// P1689:          "logical-name": "M"
// P1689:      "requires": [
// P1689-NEXT:   {
// P1689-NEXT:     "logical-name": "h.h",
// P1689-NEXT:     "lookup-method": "include-quote",
// P1689-NEXT:     "source-path": "[[PREFIX]]/h.h",
// P1689-NEXT:     "unique-on-source-path": true
// P1689-NEXT:   },
// P1689-NEXT:   {
// P1689-NEXT:     "logical-name": "w.h",
// P1689-NEXT:     "lookup-method": "include-angle",
// P1689-NEXT:     "source-path": "[[PREFIX]]/inc/w.h",
// P1689-NEXT:     "unique-on-source-path": true
// P1689-NEXT:   }
// P1689-NEXT: ]

// FULL-NOT: "named-module-deps"
// FULL: "named-module": "M"
// FULL-NOT: "named-module-deps"

//--- a.cppm
export module M;
import "h.h";
import <w.h>;

//--- h.h
int h();

//--- inc/w.h
int w();
