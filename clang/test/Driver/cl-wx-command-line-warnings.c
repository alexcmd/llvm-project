// CL-WX-CMDLINE: cl 19.44 keeps command line warnings (D9002, D9007, D9014,
// D9025, D9026, D9035) warnings under /WX (observed: exit 0).

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /WX /Gm /Ze /foo /W5 /wd99999 /O1 /O2 -### -- %s 2>&1 | FileCheck %s
// CHECK-DAG: warning: D9035 : option 'Gm' is deprecated
// CHECK-DAG: warning: D9035 : option 'Ze' is deprecated
// CHECK-DAG: warning: D9002 : skipping unrecognized option '/foo'
// CHECK-DAG: warning: D9014 : invalid value '5' for '/W'; assuming '1'
// CHECK-DAG: warning: D9014 : invalid value '99999' for '/wd'; assuming '5999'
// CHECK-DAG: warning: D9025 : overriding '/O1' with '/O2'
// CHECK-DAG: warning: D9007 : '/Gm' requires '/Zi or /ZI'; option ignored
// CHECK-NOT: error:
