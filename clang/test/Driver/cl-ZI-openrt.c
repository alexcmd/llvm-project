// CL-ZI: /ZI gives debug information as /Zi does (no "argument unused", and
// /JMC is not left without debug information).
// RUN: %clang_cl --target=x86_64-windows-msvc /ZI /c -### -- %s 2>&1 | FileCheck -check-prefix=ZI %s
// RUN: %clang_cl --target=x86_64-windows-msvc /ZI /JMC /c -### -- %s 2>&1 | FileCheck -check-prefix=ZI %s
// ZI-NOT: argument unused
// ZI-NOT: D9007
// ZI: "-gcodeview"
// ZI: "-debug-info-kind=
