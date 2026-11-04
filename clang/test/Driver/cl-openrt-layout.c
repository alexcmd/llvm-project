// BIN-RELATIVE-LOOKUP: an openrt-<ver> layout around the driver
// (<bin>\..\manifest.json) supplies <root>\include and <root>\lib\<arch> as if
// /vctoolsdir:<root> were given; manifest.json sdk.root/sdk.version supply the
// Windows SDK when /winsdkdir is absent.

// RUN: rm -rf %t
// RUN: split-file %s %t
// RUN: mkdir -p %t/openrt-0.8/bin %t/openrt-0.8/manifest.json

// Layout found: include and lib\x64 from the layout root.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=LAYOUT
// LAYOUT: "-internal-isystem" "[[ROOT:[^"]*]]{{/|\\\\}}openrt-1.0{{/|\\\\}}include"
// LAYOUT: "-libpath:[[ROOT]]{{/|\\\\}}openrt-1.0{{/|\\\\}}lib{{/|\\\\}}x64"

// Empty %INCLUDE%/%LIB% count as unset.
// RUN: env -u EXTERNAL_INCLUDE -u VCToolsInstallDir -u VCINSTALLDIR INCLUDE= LIB= \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=LAYOUT

// Other arch: lib\<arch> as in the VS2017+ layout.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=aarch64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=ARM64
// ARM64: "-libpath:{{[^"]*}}openrt-1.0{{/|\\\\}}lib{{/|\\\\}}arm64"

// %INCLUDE% or %LIB% set: the layout is not used.
// RUN: env -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR INCLUDE=%t/envinc \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=NOLAYOUT
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u VCToolsInstallDir -u VCINSTALLDIR LIB=%t/envlib \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=NOLAYOUT
// NOLAYOUT-NOT: openrt-1.0{{/|\\\\}}include"
// NOLAYOUT-NOT: openrt-1.0{{/|\\\\}}lib{{/|\\\\}}x64"

// /vctoolsdir wins over the layout.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /vctoolsdir %t/vc -fuse-ld=lld -### -- %t/foo.c 2>&1 \
// RUN:   | FileCheck %s --check-prefix=VCTOOLSDIR
// VCTOOLSDIR-NOT: openrt-1.0{{/|\\\\}}include"
// VCTOOLSDIR: "-internal-isystem" "{{[^"]*}}{{/|\\\\}}vc{{/|\\\\}}include"
// VCTOOLSDIR-NOT: openrt-1.0{{/|\\\\}}lib
// VCTOOLSDIR: "-libpath:{{[^"]*}}{{/|\\\\}}vc{{/|\\\\}}lib{{/|\\\\}}x64"
// VCTOOLSDIR-NOT: openrt-1.0{{/|\\\\}}lib

// No manifest.json (or manifest.json is a directory): no layout.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-0.9/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=NOMANIFEST
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-0.8/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=NOMANIFEST
// NOMANIFEST-NOT: openrt-0.{{[89]}}{{/|\\\\}}include"
// NOMANIFEST-NOT: openrt-0.{{[89]}}{{/|\\\\}}lib

// manifest.json sdk.root (relative to the layout root) and sdk.version select
// the Windows SDK and UCRT (no stdlib.h in <root>\include).
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-2.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=SDK
// SDK: "-internal-isystem" "[[ROOT:[^"]*]]{{/|\\\\}}openrt-2.0{{/|\\\\}}include"
// SDK: "-internal-isystem" "[[ROOT]]{{/|\\\\}}openrt-2.0{{/|\\\\}}atlmfc{{/|\\\\}}include"
// SDK: "-internal-isystem" "[[ROOT]]{{/|\\\\}}kits{{/|\\\\}}10{{/|\\\\}}Include{{/|\\\\}}10.0.11111.0{{/|\\\\}}ucrt"
// SDK: "-internal-isystem" "[[ROOT]]{{/|\\\\}}kits{{/|\\\\}}10{{/|\\\\}}Include{{/|\\\\}}10.0.11111.0{{/|\\\\}}shared"
// SDK: "-internal-isystem" "[[ROOT]]{{/|\\\\}}kits{{/|\\\\}}10{{/|\\\\}}Include{{/|\\\\}}10.0.11111.0{{/|\\\\}}um"
// SDK: "-internal-isystem" "[[ROOT]]{{/|\\\\}}kits{{/|\\\\}}10{{/|\\\\}}Include{{/|\\\\}}10.0.11111.0{{/|\\\\}}winrt"
// SDK: "-libpath:[[ROOT]]{{/|\\\\}}openrt-2.0{{/|\\\\}}lib{{/|\\\\}}x64"
// SDK: "-libpath:[[ROOT]]{{/|\\\\}}openrt-2.0{{/|\\\\}}atlmfc{{/|\\\\}}lib{{/|\\\\}}x64"
// SDK: "-libpath:[[ROOT]]{{/|\\\\}}kits{{/|\\\\}}10{{/|\\\\}}Lib{{/|\\\\}}10.0.11111.0{{/|\\\\}}ucrt{{/|\\\\}}x64"
// SDK: "-libpath:[[ROOT]]{{/|\\\\}}kits{{/|\\\\}}10{{/|\\\\}}Lib{{/|\\\\}}10.0.11111.0{{/|\\\\}}um{{/|\\\\}}x64"

// /winsdkdir wins over manifest.json sdk.* (version is not taken either).
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-2.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /winsdkdir %t/otherkits -fuse-ld=lld -### -- %t/foo.c 2>&1 \
// RUN:   | FileCheck %s --check-prefix=WINSDKDIR
// WINSDKDIR: "-internal-isystem" "{{[^"]*}}openrt-2.0{{/|\\\\}}include"
// WINSDKDIR-NOT: 10.0.11111.0
// WINSDKDIR: "-internal-isystem" "{{[^"]*}}otherkits{{/|\\\\}}Include{{/|\\\\}}10.0.33333.0{{/|\\\\}}um"
// WINSDKDIR-NOT: 10.0.11111.0
// WINSDKDIR: "-libpath:{{[^"]*}}otherkits{{/|\\\\}}Lib{{/|\\\\}}10.0.33333.0{{/|\\\\}}um{{/|\\\\}}x64"

// Malformed manifest.json: layout still used, no SDK info from it.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-3.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fuse-ld=lld -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=BADJSON
// BADJSON: "-internal-isystem" "{{[^"]*}}openrt-3.0{{/|\\\\}}include"
// BADJSON-NOT: kits{{/|\\\\}}10
// BADJSON: "-libpath:{{[^"]*}}openrt-3.0{{/|\\\\}}lib{{/|\\\\}}x64"
// BADJSON-NOT: kits{{/|\\\\}}10

//--- openrt-1.0/manifest.json
{"version": "1.0"}
//--- openrt-1.0/bin/.keep
//--- openrt-1.0/include/stdlib.h
//--- openrt-1.0/lib/x64/.keep
//--- openrt-0.9/bin/.keep
//--- openrt-0.9/include/stdlib.h
//--- openrt-2.0/manifest.json
{"version": "2.0", "sdk": {"root": "../kits/10", "version": "10.0.11111.0"}}
//--- openrt-2.0/bin/.keep
//--- openrt-2.0/include/vcruntime.h
//--- kits/10/Include/10.0.11111.0/um/windows.h
//--- kits/10/Include/10.0.22222.0/um/windows.h
//--- otherkits/Include/10.0.33333.0/ucrt/stdlib.h
//--- otherkits/Include/10.0.33333.0/um/windows.h
//--- openrt-3.0/manifest.json
{"version": "3.0", "sdk": {"root":
//--- openrt-3.0/bin/.keep
//--- openrt-3.0/include/stdlib.h
//--- vc/include/stdlib.h
//--- foo.c
int main(void) { return 0; }
