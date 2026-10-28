// -fms-predeclare-eh-types force-includes the resource header declaring the
// C++ EH types cl.exe pre-injects; C++ only, and ahead of user /FI includes.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   /TP /FIuser.h -### -c -- %s 2>&1 | FileCheck --check-prefix=ON %s
// RUN: %clang --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   -x c++ -include user.h -### -c %s 2>&1 | FileCheck --check-prefix=ON %s
// ON: "-include" "__msvc_eh_predef.h" {{.*}}"-include" "user.h"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /TP -### -c -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   -fno-ms-predeclare-eh-types /TP -### -c -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   /TC -### -c -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// OFF-NOT: __msvc_eh_predef.h
// OFF-NOT: unused
