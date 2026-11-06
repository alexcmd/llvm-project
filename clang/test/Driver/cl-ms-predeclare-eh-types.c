// The resource header declaring the C++ EH and RTTI types cl.exe pre-injects is
// force-included for C++ inputs: by default in cl mode, with
// -fms-predeclare-eh-types elsewhere; ahead of user /FI includes; not in C and
// not when only preprocessing.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /TP /FIuser.h -### -c -- %s \
// RUN:   2>&1 | FileCheck --check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   /TP /FIuser.h -### -c -- %s 2>&1 | FileCheck --check-prefix=ON %s
// RUN: %clang_cl --target=i686-pc-windows-msvc /TP /FIuser.h /Yupch.h -### \
// RUN:   -c -- %s 2>&1 | FileCheck --check-prefix=ON %s
// RUN: %clang --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   -x c++ -include user.h -### -c %s 2>&1 | FileCheck --check-prefix=ON %s
// ON: "-include" "__msvc_eh_predef.h" {{.*}}"-include" "user.h"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /TP -fno-ms-predeclare-eh-types \
// RUN:   -### -c -- %s 2>&1 | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   -fno-ms-predeclare-eh-types /TP -### -c -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /TC -### -c -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /TP /E -### -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /TP /P -### -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// RUN: %clang --target=x86_64-pc-windows-msvc -x c++ -### -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// OFF-NOT: __msvc_eh_predef.h
// OFF-NOT: argument unused

// A preprocessed C++ file gets the types when it is compiled.
// RUN: rm -rf %t && mkdir %t && echo > %t/a.ii
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c -- %t/a.ii 2>&1 \
// RUN:   | FileCheck --check-prefix=ON-PP %s
// ON-PP: "-include" "{{.*}}include{{/|\\\\}}__msvc_eh_predef.h"
