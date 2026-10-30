// cl.exe: an unknown /option is D9002 (warning, go on) unless /options:strict
// makes it D8043 (error, exit 2). Near misses (/EBUG) get no "did you mean".
// Probed with cl 14.51.36231: /options:strict /bogus /EBUG gives D8043 for each.

// RUN: not %clang_cl /options:strict /bogusopt /EBUG -c -- %s 2>&1 | FileCheck --check-prefix=STRICT %s
// STRICT: error: D8043 : unknown option '/bogusopt'
// STRICT: error: D8043 : unknown option '/EBUG'
// STRICT-NOT: did you mean

// /options:strict counts wherever it appears on the command line.
// RUN: not %clang_cl /bogusopt /options:strict -c -- %s 2>&1 | FileCheck --check-prefix=STRICT1 %s
// STRICT1: error: D8043 : unknown option '/bogusopt'

// RUN: %clang_cl /options:strict -c -### -- %s 2>&1 | FileCheck --check-prefix=OK %s
// OK-NOT: D8043
// OK: "-cc1"

// RUN: %clang_cl /bogusopt /EBUG -c -### -- %s 2>&1 | FileCheck --check-prefix=LAX %s
// LAX: warning: D9002 : ignoring unknown option '/bogusopt'
// LAX: warning: D9002 : ignoring unknown option '/EBUG'
// LAX-NOT: did you mean
// LAX-NOT: no such file
// LAX: "-cc1"
