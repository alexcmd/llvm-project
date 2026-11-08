// openrt CL-IDENTITY: cl defines none of the clang identity macros;
// /clang:-fno-ms-clang-identity leaves them out (cl mode keeps them by default
// for now, CLModeIdentity). Without them, as cl 14.51, clang-cl predefines _MSVC_WARNING_LEVEL (the final /W level as
// a long: 0L for /W0 and /w, 100L for /Wall, 1L by default) and _MSVC_MT
// for /MT only.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /clang:-fno-ms-clang-identity -- %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=NOID,W1,MT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=ID %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -fno-ms-clang-identity -fms-clang-identity -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=ID %s
// RUN: %clang --target=x86_64-pc-windows-msvc -c -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=ID %s
// RUN: %clang --target=x86_64-pc-windows-msvc -c -### -fno-ms-clang-identity %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NOIDGNU %s

// NOID-DAG: "-fno-ms-clang-identity"
// ID-NOT: "-fno-ms-clang-identity"
// ID-NOT: "-D_MSVC_WARNING_LEVEL
// ID-NOT: "-D_MSVC_MT
// NOIDGNU: "-fno-ms-clang-identity"
// W1-DAG: "-D_MSVC_WARNING_LEVEL=1L"
// MT-DAG: "-D_MSVC_MT=1"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -fno-ms-clang-identity /W4 /MD -- %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=W4,NOMT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -fno-ms-clang-identity /W2 /W4 /MTd -- %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=W4,NOMT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -fno-ms-clang-identity /W3 /MDd -- %s 2>&1 \
// RUN:   | FileCheck --check-prefixes=W3,NOMT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -fno-ms-clang-identity /Wall -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=WALL %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -fno-ms-clang-identity /W0 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=W0 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -fno-ms-clang-identity /W4 /w -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=W0 %s
// W4: "-D_MSVC_WARNING_LEVEL=4L"
// W3: "-D_MSVC_WARNING_LEVEL=3L"
// WALL: "-D_MSVC_WARNING_LEVEL=100L"
// W0: "-D_MSVC_WARNING_LEVEL=0L"
// NOMT-NOT: "-D_MSVC_MT
