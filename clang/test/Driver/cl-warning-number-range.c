// openrt CL-WARNING-NUMBER-RANGE: cl 14.51 takes warning numbers 4000..5999
// for /we /wd /wo /w<l>; any other is D9014 (a warning) and stands for 5999.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /we9999 /wd3999 /w19999 /wo6000 -- %s 2>&1 | FileCheck %s
// CHECK: warning: D9014 : invalid value '9999' for '/we'; assuming '5999'
// CHECK: warning: D9014 : invalid value '3999' for '/wd'; assuming '5999'
// CHECK: warning: D9014 : invalid value '9999' for '/w1'; assuming '5999'
// CHECK: warning: D9014 : invalid value '6000' for '/wo'; assuming '5999'
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /we5999 /wd4000 -- %s 2>&1 | FileCheck --check-prefix=OK %s
// OK-NOT: D9014
