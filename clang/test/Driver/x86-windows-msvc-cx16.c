// x86-64 MSVC targets enable CMPXCHG16B by default (required by 64-bit
// Windows; no __atomic_compare_exchange_16 libcall exists there).

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c -- %s 2>&1 | FileCheck %s --check-prefix=CX16
// RUN: %clang --target=x86_64-pc-windows-msvc -### -c %s 2>&1 | FileCheck %s --check-prefix=CX16
// RUN: %clang --target=x86_64-pc-windows-msvc -mno-cx16 -### -c %s 2>&1 | FileCheck %s --check-prefix=OFF
// RUN: %clang_cl --target=i686-pc-windows-msvc -### -c -- %s 2>&1 | FileCheck %s --check-prefix=NONE
// RUN: %clang --target=x86_64-w64-windows-gnu -### -c %s 2>&1 | FileCheck %s --check-prefix=NONE
// RUN: %clang --target=x86_64-unknown-linux-gnu -### -c %s 2>&1 | FileCheck %s --check-prefix=NONE

// CX16: "-target-feature" "+cx16"
// OFF-NOT: "+cx16"
// OFF: "-target-feature" "-cx16"
// NONE-NOT: "+cx16"
