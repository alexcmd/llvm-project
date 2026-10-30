// CL and _CL_ are read like a command line: '@file' is expanded there too
// (cl.exe does this), and a missing file is error D8022 with exit code 2.
// Order as cl.exe: CL, then the command line, then _CL_ (last one wins).

// RUN: printf '%%s\n' '/DFROMRSP=1' > %t.rsp
// RUN: env CL="@%t.rsp" %clang_cl -### -c -- %s 2>&1 | FileCheck %s --check-prefix=CL-RSP
// CL-RSP: "-D" "FROMRSP=1"

// RUN: env _CL_="@%t.rsp" %clang_cl -### -c -- %s 2>&1 | FileCheck %s --check-prefix=CL-RSP

// RUN: env CL="/DENV=7" %clang_cl -### -c /DENV=3 -- %s 2>&1 | FileCheck %s --check-prefix=ORDER
// RUN: env CL="/DENV=7" _CL_="/DENV=9" %clang_cl -### -c /DENV=3 -- %s 2>&1 | FileCheck %s --check-prefix=ORDER
// ORDER:      "-D" "ENV=7"
// ORDER-NEXT: "-D" "ENV=3"
// ORDER-NEXT: "-D" "ENV=9"

// RUN: not env CL="@%t.missing" %clang_cl -### -c -- %s 2>&1 | FileCheck %s --check-prefix=MISSING
// RUN: not env _CL_="@%t.missing" %clang_cl -### -c -- %s 2>&1 | FileCheck %s --check-prefix=MISSING
// MISSING: clang-cl: error: D8022 : cannot open '{{.*}}missing'

// RUN: not %clang_cl -### -c @%t.missing -- %s 2>&1 | FileCheck %s --check-prefix=CMDMISSING
// CMDMISSING: clang-cl: error: D8022 : cannot open '{{.*}}missing'
