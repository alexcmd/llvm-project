// CL-OPT-OVERRIDE: cl 19.44 (observed) warns D9025 when a later option of the
// same kind replaces an earlier different one, once per change in command
// line order, and D9007 for an option that has no effect without another.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /O1 /O2 /MD /MT /MD /GR /GR- /W3 /W4 /Zi /Z7 /EHsc /EHa /std:c++17 /std:c++20 /fp:fast /fp:precise /Zp /Zp8 /permissive- /permissive /GL /GL- -### -- %s 2>&1 | FileCheck --check-prefix=OVR %s
// OVR: warning: D9025 : overriding '/O1' with '/O2'
// OVR: warning: D9025 : overriding '/MD' with '/MT'
// OVR: warning: D9025 : overriding '/MT' with '/MD'
// OVR: warning: D9025 : overriding '/GR' with '/GR-'
// OVR: warning: D9025 : overriding '/W3' with '/W4'
// OVR: warning: D9025 : overriding '/Zi' with '/Z7'
// OVR: warning: D9025 : overriding '/EHs' with '/EHa'
// OVR: warning: D9025 : overriding '/std:c++17' with '/std:c++20'
// OVR: warning: D9025 : overriding '/fp:fast' with '/fp:precise'
// OVR: warning: D9025 : overriding '/Zp' with '/Zp8'
// OVR: warning: D9025 : overriding '/permissive-' with '/permissive'
// OVR: warning: D9025 : overriding '/GL' with '/GL-'
// OVR-NOT: D9025
// OVR-NOT: D9007

// A warning also under /WX (cl: exit 0).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /WX /O1 /O2 /EHc -### -- %s 2>&1 | FileCheck --check-prefix=WX %s
// WX: warning: D9025 : overriding '/O1' with '/O2'
// WX: warning: D9007 : '/EHc' requires '/EHa, /EHs or /GX'; option ignored
// WX-NOT: error:

// More kinds: /O letters (/Ox is /O2: no warning for /O2i- after /Ox), /EH letters.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Od /Ox /Ot /Os /O2i- /Oi /EHs- /EHsc /EHc- /W4 /Wall /W3 -### -- %s 2>&1 | FileCheck --check-prefix=OVR2 %s
// OVR2: warning: D9025 : overriding '/Od' with '/Ox'
// OVR2: warning: D9025 : overriding '/Ot' with '/Os'
// OVR2: warning: D9025 : overriding '/Oi-' with '/Oi'
// OVR2: warning: D9025 : overriding '/EHs-' with '/EHs'
// OVR2: warning: D9025 : overriding '/EHc' with '/EHc-'
// OVR2: warning: D9025 : overriding '/W4' with '/W3'
// OVR2-NOT: D9025

// Silent: equal options, /O2 /Ox, /O1 /Os, /Os /O2, /w /W0, /GL- /GL,
// /EHa /EHs-, /EHs /EHa-, /W4 /Wall.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /O2 /Ox /O2 /MT /MT /W4 /Wall /GL- /GL /EHa /EHs- /Zi /Zi -### -- %s 2>&1 | FileCheck --check-prefix=SILENT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /O1 /Os /EHs /EHa- /w /W0 -### -- %s 2>&1 | FileCheck --check-prefix=SILENT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Os /O2 -### -- %s 2>&1 | FileCheck --check-prefix=SILENT %s
// SILENT-NOT: D9025
// SILENT-NOT: D9007

// D9007.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /EHc /Gm -### -- %s 2>&1 | FileCheck --check-prefix=REQ %s
// REQ: warning: D9035 : option 'Gm' is deprecated
// REQ: warning: D9007 : '/EHc' requires '/EHa, /EHs or /GX'; option ignored
// REQ: warning: D9007 : '/Gm' requires '/Zi or /ZI'; option ignored
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /EHsc /Gm /Zi -### -- %s 2>&1 | FileCheck --check-prefix=NOREQ %s
// NOREQ-NOT: D9007

// Not in the clang driver.
// RUN: %clang --target=x86_64-pc-windows-msvc -c -O1 -O2 -### -- %s 2>&1 | FileCheck --check-prefix=GNU %s
// GNU-NOT: D9025
