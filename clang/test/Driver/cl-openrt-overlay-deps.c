// REAL-DEP-PATHS: /showIncludes and /sourceDependencies report a header
// reached through a VFS overlay (external names) by its real on-disk path, not
// by the virtual one, and list the overlay the driver loaded from the openrt
// layout (-fdepfile-entry) as an input. Build tools stat what they get, and the
// virtual path does not exist (ninja deps = msvc would rebuild every time).

// RUN: rm -rf %t && split-file %s %t
// RUN: sed -e 's|VROOT|%/t/virt/include|g' %t/m.json > %t/openrt-1.0/manifest.json
// RUN: sed -e 's|VROOT|%/t/virt/include|g' %t/names.yaml > %t/openrt-1.0/include/openrt-names.yaml
// RUN: mkdir -p %t/openrt-1.0/bin

// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /Zs /showIncludes -- %t/foo.c 2>&1 \
// RUN:   | FileCheck %s --check-prefix=SHOW --implicit-check-not=virt
// SHOW:      Note: including file: {{.*}}openrt-1.0\include\openrt-names.yaml
// SHOW-NEXT: Note: including file: {{.*}}openrt-1.0\include\openapi.h
// SHOW-NEXT: Note: including file:  {{.*}}openrt-1.0\include\openinner.h
// SHOW-NEXT: Note: including file: {{.*}}local.h

// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/openrt-1.0/bin --target=x86_64-pc-windows-msvc \
// RUN:   /Zs /sourceDependencies %t/foo.json -- %t/foo.c
// RUN: FileCheck %s --check-prefix=JSON --implicit-check-not=virt --input-file=%t/foo.json
// JSON:      "Includes": [
// JSON-NEXT:   "{{.*}}openrt-1.0\\include\\openrt-names.yaml",
// JSON-NEXT:   "{{.*}}openrt-1.0\\include\\openapi.h",
// JSON-NEXT:   "{{.*}}openrt-1.0\\include\\openinner.h",
// JSON-NEXT:   "{{.*}}local.h"
// JSON-NEXT: ]

// An overlay given by hand: real path too; no implicit overlay entry.
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl --target=x86_64-pc-windows-msvc -fno-openrt-overlay /X \
// RUN:   -Xclang -ivfsoverlay -Xclang %t/openrt-1.0/include/openrt-names.yaml \
// RUN:   /imsvc %/t/virt/include /Zs /showIncludes -- %t/foo.c 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MANUAL --implicit-check-not=virt
// MANUAL-NOT: openrt-names.yaml
// MANUAL:      Note: including file: {{.*}}openrt-1.0\include\openapi.h
// MANUAL-NEXT: Note: including file:  {{.*}}openrt-1.0\include\openinner.h

//--- m.json
{"version": "1.0", "overlay": {"file": "include/openrt-names.yaml", "prefix": "VROOT"}}
//--- names.yaml
{ 'version': 0, 'overlay-relative': true,
  'roots': [ { 'name': 'VROOT', 'type': 'directory',
               'contents': [ { 'name': 'vapi.h', 'type': 'file',
                               'external-contents': 'openapi.h' },
                             { 'name': 'vinner.h', 'type': 'file',
                               'external-contents': 'openinner.h' } ] } ] }
//--- openrt-1.0/include/openapi.h
#include <vinner.h>
//--- openrt-1.0/include/openinner.h
int inner;
//--- local.h
int local;
//--- foo.c
#include <vapi.h>
#include "local.h"
