// openrt UE-D2-EXTWARN: UnrealBuildTool passes /d2ExtendedWarningInfo on
// every compile; cl 14.51 accepts it silently, also under /WX. Other /d2
// names stay C1007 (p2), as in cl.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /WX /c -### /d2ExtendedWarningInfo -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OK %s
// OK-NOT: C1007
// OK-NOT: argument unused
// OK: "-cc1"

// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /c -### /d2xyzzy -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=BAD %s
// BAD: C1007
