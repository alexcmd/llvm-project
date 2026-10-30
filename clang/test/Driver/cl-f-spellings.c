// /F<letter> and /W, /H spellings that cl.exe 14.51 accepts must not be
// D8021 "invalid numeric argument" (probed with cl /c, exit 0, no output).
// /FD is bare only; /Fw and /Fl take any value.

// RUN: %clang_cl /c /FD /Fw /Fl /FA /Fa%t.s /FC /Fd%t.pdb /Fe%t.exe /Fm%t.map /Fo%t.obj /Fp%t.pch /FR%t.sbr /FS /Fx /F1 /F0x100 /H0 /W0 /W4 /Wall /WX- /Wv:17 /wd4996 /we4996 -### -- %s 2>&1 | FileCheck --check-prefix=VALID %s
// RUN: %clang_cl /c /Fwx /Flx /Fw:x /Fl:y -### -- %s 2>&1 | FileCheck --check-prefix=VALID %s
// VALID: "-cc1"
// VALID-NOT: D8021

// cl.exe rejects these with D8021 (exit 2); the stack-size and /H rules stay.
// RUN: not %clang_cl /c /FDx -### -- %s 2>&1 | FileCheck --check-prefix=BAD-FD %s
// BAD-FD: D8021 : invalid numeric argument '/FDx'
// RUN: not %clang_cl /c /Fz -### -- %s 2>&1 | FileCheck --check-prefix=BAD-FZ %s
// BAD-FZ: D8021 : invalid numeric argument '/Fz'
// RUN: not %clang_cl /c /Hx -### -- %s 2>&1 | FileCheck --check-prefix=BAD-H %s
// BAD-H: D8021 : invalid numeric argument '/Hx'
