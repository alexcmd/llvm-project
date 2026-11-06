// ZC-PREPROCESSOR: /Zc:preprocessor is accepted silently (also with /WX) and
// sets _MSVC_TRADITIONAL to 0; /Zc:preprocessor- restores it.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-compatibility-version=19.44 \
// RUN:   /WX /Zc:preprocessor /E /clang:-dM -- %s 2>&1 | FileCheck %s --check-prefix=CONF
// CONF-NOT: warning
// CONF-NOT: error
// CONF: #define _MSVC_TRADITIONAL 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-compatibility-version=19.44 \
// RUN:   /WX /Zc:preprocessor /Zc:preprocessor- /E /clang:-dM -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRAD
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-compatibility-version=19.44 \
// RUN:   /E /clang:-dM -- %s 2>&1 | FileCheck %s --check-prefix=TRAD
// TRAD-NOT: warning
// TRAD: #define _MSVC_TRADITIONAL 1

// CLANG-CL-GAP(Qspectre): accepted with a remark that /WX does not promote.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /WX /Qspectre /Qspectre-load \
// RUN:   /Zs -- %s 2>&1 | FileCheck %s --check-prefix=SPECTRE
// SPECTRE-NOT: warning
// SPECTRE-NOT: error
// SPECTRE: remark: '/Qspectre' accepted without effect
// SPECTRE: remark: '/Qspectre-load' accepted without effect

// RSP-SEPARATE-ARG: in a response file the value of a separate-argument
// option may sit on the next line (no assertion, no missing-argument error).
// RUN: rm -rf %t && mkdir -p %t
// RUN: echo /c > %t/a.rsp
// RUN: echo /sourceDependencies >> %t/a.rsp
// RUN: echo deps.json >> %t/a.rsp
// RUN: echo /reference >> %t/a.rsp
// RUN: echo x.ifc >> %t/a.rsp
// RUN: %clang_cl --target=x86_64-pc-windows-msvc @%t/a.rsp -### -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=RSP
// RSP-NOT: error
// RSP: "-sourceDependencies-json" "deps.json"

int x;
