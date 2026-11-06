// VFS-OVERLAY-AUTOLOAD: manifest.json "overlay" of an openrt-<ver> layout
// around the driver names a VFS overlay (relative to the layout root) and the
// virtual include dir it is rooted at. The driver passes -ivfsoverlay, adds the
// virtual dir as a system include dir and lists the overlay as a dependency
// (-fdepfile-entry). No marker, no "overlay", a missing overlay file or
// -fno-openrt-overlay: nothing is added.

// REQUIRES: x86-registered-target

// RUN: rm -rf %t && split-file %s %t
// RUN: sed -e 's|VROOT|%/t/virt/include|g' %t/m.json > %t/openrt-1.0/manifest.json
// RUN: sed -e 's|VROOT|%/t/virt/include|g' %t/names.yaml > %t/openrt-1.0/include/openrt-names.yaml
// RUN: mkdir -p %t/openrt-1.0/bin %t/openrt-2.0/bin %t/openrt-3.0/bin %t/noman/bin
// RUN: cp %t/openrt-1.0/manifest.json %t/noman/manifest.json.txt
// RUN: sed -e 's|VROOT|%/t/virt/include|g' %t/m.json > %t/openrt-3.0/manifest.json

// Overlay declared: loaded, virtual dir searched, overlay is a dependency.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=ON
// ON: "-ivfsoverlay" "[[YAML:[^"]*openrt-1.0[/\\]+include[/\\]+openrt-names.yaml]]"
// ON-SAME: "-fdepfile-entry=[[YAML]]"
// ON: "-internal-isystem" "{{[^"]*}}virt{{[/\\]+}}include"

// Also when %INCLUDE% / /vctoolsdir pick another toolchain, and with /X.
// RUN: env -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR INCLUDE=%t/envinc \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=ON
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /vctoolsdir %t/vc /X -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=ON

// The clang driver in MSVC mode does the same.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -c %t/foo.c 2>&1 | FileCheck %s --check-prefix=ON

// Opt-out, no "overlay" field, declared file missing, no marker.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fno-openrt-overlay -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=OFF
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-2.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=OFF
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-3.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=OFF
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/noman/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=OFF
// OFF-NOT: -ivfsoverlay
// OFF-NOT: virt{{[/\\]+}}include"
// OFF-NOT: -fdepfile-entry

// The API name resolves to the renamed file; __FILE__ is the real path.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /E -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=PP
// PP: int from_openapi_h;
// PP: const char *where = "{{.*}}openrt-1.0{{[/\\]+}}include{{[/\\]+}}openapi.h";
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   not %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   -fno-openrt-overlay /E -- %t/foo.c 2>&1 | FileCheck %s --check-prefix=NOTFOUND
// NOTFOUND: 'vapi.h' file not found

// -MD depfile: real path of the renamed header, plus the overlay itself.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /Zs /clang:-MD /clang:-MF%t/foo.d -- %t/foo.c
// RUN: FileCheck %s --check-prefix=DEP --input-file=%t/foo.d
// DEP: openrt-names.yaml
// DEP: openapi.h
// DEP-NOT: vapi.h

// PCH create and use with the same auto-loaded overlay.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /Ycpch.h /FIpch.h /Fp%t/p.pch /Fo%t/pch.obj /c -- %t/pch.cpp
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /Yupch.h /FIpch.h /Fp%t/p.pch /Fo%t/use.obj /c -- %t/use.cpp
// RUN: llvm-objdump -t %t/use.obj | FileCheck %s --check-prefix=PCH
// PCH: use_openapi

//--- m.json
{"version": "1.0", "overlay": {"file": "include/openrt-names.yaml", "prefix": "VROOT"}}
//--- names.yaml
{ 'version': 0, 'overlay-relative': true,
  'roots': [ { 'name': 'VROOT', 'type': 'directory',
               'contents': [ { 'name': 'vapi.h', 'type': 'file',
                               'external-contents': 'openapi.h' } ] } ] }
//--- openrt-1.0/include/openapi.h
int from_openapi_h;
const char *where = __FILE__;
//--- openrt-2.0/manifest.json
{"version": "2.0"}
//--- openrt-2.0/include/openrt-names.yaml
//--- openrt-3.0/include/.keep
//--- vc/include/.keep
//--- foo.c
#include <vapi.h>
//--- pch.h
#include <vapi.h>
//--- pch.cpp
//--- use.cpp
int use_openapi() { return from_openapi_h; }
