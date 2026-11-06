// cl.exe 14.51 argument-value errors (probes_cl_decisions.md section 1; tasks
// 10 and 12). Exit codes: D8xxx and C1007 in c1 are 2; C1007 in p2 and C1083
// for /ZX are 1 (lit checks the text; the exit code is checked by probe).
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /HEAP -- %s 2>&1 | FileCheck --check-prefix=D8021-HEAP %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /WEAKORDER -- %s 2>&1 | FileCheck --check-prefix=D8021-WEAK %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /Fv -- %s 2>&1 | FileCheck --check-prefix=D8021-FV %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /FORCE -- %s 2>&1 | FileCheck --check-prefix=D8021-FORCE %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /Wv -- %s 2>&1 | FileCheck --check-prefix=D8021-WV %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /WS -- %s 2>&1 | FileCheck --check-prefix=D8021-WS %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /WT -- %s 2>&1 | FileCheck --check-prefix=D8021-WT %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /WarnOnGlobals -- %s 2>&1 | FileCheck --check-prefix=D8021-WARN %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /WINMDFILE -- %s 2>&1 | FileCheck --check-prefix=D8021-WINMD %s
// D8021-HEAP: D8021 : '/HEAP' does not hold a valid numeric value
// D8021-WEAK: D8021 : '/WEAKORDER' does not hold a valid numeric value
// D8021-FV: D8021 : '/Fv' does not hold a valid numeric value
// D8021-FORCE: D8021 : '/FORCE' does not hold a valid numeric value
// D8021-WV: D8021 : '/Wv' does not hold a valid numeric value
// D8021-WS: D8021 : '/WS' does not hold a valid numeric value
// D8021-WT: D8021 : '/WT' does not hold a valid numeric value
// D8021-WARN: D8021 : '/WarnOnGlobals' does not hold a valid numeric value
// D8021-WINMD: D8021 : '/WINMDFILE' does not hold a valid numeric value

// Bare forms of documented options: D8004.
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /W -- %s 2>&1 | FileCheck --check-prefix=D8004-W %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /H -- %s 2>&1 | FileCheck --check-prefix=D8004-H %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /V -- %s 2>&1 | FileCheck --check-prefix=D8004-V %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /Zm -- %s 2>&1 | FileCheck --check-prefix=D8004-ZM %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /sourcelink -- %s 2>&1 | FileCheck --check-prefix=D8004-SL %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /Ft -- %s 2>&1 | FileCheck --check-prefix=D8004-FT %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /Yl -- %s 2>&1 | FileCheck --check-prefix=C1007-YL %s
// D8004-W: D8004 : '/W' is missing its value
// D8004-H: D8004 : '/H' is missing its value
// D8004-V: D8004 : '/V' is missing its value
// D8004-ZM: D8004 : '/Zm' is missing its value
// D8004-SL: D8004 : '/sourcelink' is missing its value
// D8004-FT: D8004 : '/Ft' is missing its value
// C1007-YL: c1: fatal error C1007: switch '-Yl' is not known to 'c1'

// C file only: /ZW and /clr are D8048 and D8045.
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /ZW -- %s 2>&1 | FileCheck --check-prefix=D8048 %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /clr -- %s 2>&1 | FileCheck --check-prefix=D8045 %s
// D8048: D8048 : C source '{{.*}}' cannot be built with the /ZW option
// D8045: D8045 : C source '{{.*}}' cannot be built with the /clr option

// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /ZX -- %s 2>&1 | FileCheck --check-prefix=C1083-ZX %s
// C1083-ZX: fatal error C1083: compiler-generated file
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc -### /c /guard -- %s 2>&1 | FileCheck --check-prefix=GUARD %s
// GUARD: guardcfw.h

// Deprecated letters warn (D9035) and the option still works.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /c /VERBOSE -- %s 2>&1 | FileCheck --check-prefix=D9035-V %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /c /Ze -- %s 2>&1 | FileCheck --check-prefix=D9035-ZE %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /c /o out.obj -- %s 2>&1 | FileCheck --check-prefix=D9035-O %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /c /H31 -- %s 2>&1 | FileCheck --check-prefix=D9035-H %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /c /W5 -- %s 2>&1 | FileCheck --check-prefix=D9014 %s
// D9035-V: D9035 : option 'V' is deprecated
// D9035-ZE: D9035 : option 'Ze' is deprecated
// D9035-O: D9035 : option 'o' is deprecated
// D9035-H: D9035 : option 'H' is deprecated
// D9014: D9014 : level '5' is out of range for '/W'; using level '1'

// Accepted without a diagnostic: /Ft<dir>, /Yl<sym>, /Wv:<ver>, /WL-, /H<n>
// is above; /d2Zi is accepted (crashes in cl, GAP).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### /c /Ftdir /Ylfoo /Wv:17 /WL- /d2Zi -- %s 2>&1 | FileCheck --check-prefix=NOWARN %s
// NOWARN-NOT: warning
