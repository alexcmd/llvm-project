// REQUIRES: system-windows
// openrt UE-MSCVER-SHIM: in UBT's clean environment (no VC variables) the
// _MSC_VER comes from the toolset of the first explicit include dir holding
// the VC headers (/X /external:I <toolset>\INCLUDE): that toolset's cl.exe,
// else the family of its dir name 14.NN.* (14.50 -> cl 19.50.35739, 14.51 ->
// 19.51.36260, as measured), ahead of VS-instance discovery. A driver
// installed in a toolset (<root>\bin\Hostx64\x64\cl.exe) uses that toolset
// the same way (tests/ue/repros/r8 shim run).

// RUN: rm -rf %t && mkdir -p %t/14.50.11111/include %t/14.51.22222/include %t/other
// RUN: echo "" > %t/14.50.11111/include/vcruntime.h
// RUN: echo "" > %t/14.51.22222/include/vcruntime.h
// RUN: env -u VCToolsInstallDir -u VCINSTALLDIR -u INCLUDE %clang_cl --target=x86_64-pc-windows-msvc \
// RUN:   /c -### /X /external:I %t/other /external:I %t/14.50.11111/include -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=V1450 %s
// V1450: "-fms-compatibility-version=19.50.35739"
// RUN: env -u VCToolsInstallDir -u VCINSTALLDIR -u INCLUDE %clang_cl --target=x86_64-pc-windows-msvc \
// RUN:   /c -### /X /external:I %t/14.51.22222/include -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=V1451 %s
// V1451: "-fms-compatibility-version=19.51.36260"
// An explicit version still wins.
// RUN: env -u VCToolsInstallDir -u VCINSTALLDIR -u INCLUDE %clang_cl --target=x86_64-pc-windows-msvc \
// RUN:   /c -### -fms-compatibility-version=19.40 /external:I %t/14.51.22222/include -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=EXPL %s
// EXPL: "-fms-compatibility-version=19.40"
