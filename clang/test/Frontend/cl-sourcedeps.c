// cl.exe /showIncludes and /sourceDependencies spell includes the same way:
// a quoted include next to the source is an absolute path (Windows host only;
// the drive letter is not checked), an /I include is the search dir as given.
// Probed with cl 14.51.36231 on the same tree (Inputs/cl-deps).
// REQUIRES: system-windows

// RUN: %clang_cl /c /showIncludes /I %S/Inputs/cl-deps/inc -o %t.obj -- %S/Inputs/cl-deps/main.c 2>&1 | FileCheck --check-prefix=SHOW %s
// SHOW: Note: including file: {{[^.].*}}a.h
// SHOW-NEXT: Note: including file:  {{[^.].*}}sub/c.h
// SHOW-NEXT: Note: including file: {{.*}}inc\b.h
// SHOW-NEXT: Note: including file:  {{[^.].*}}inc\d.h
// SHOW-NOT: including file: .

// RUN: %clang_cl /c /sourceDependencies %t.json /I %S/Inputs/cl-deps/inc -o %t.obj -- %S/Inputs/cl-deps/main.c
// RUN: FileCheck --input-file=%t.json --check-prefix=JSON %s
// JSON: {
// JSON:     "Version": "1.2",
// JSON:     "Data": {
// JSON:         "Source": "{{.*}}main.c",
// JSON:         "ProvidedModule": "",
// JSON:         "Includes": [
// JSON:             "{{.*}}a.h",
// JSON:             "{{.*}}sub\\c.h",
// JSON:             "{{.*}}inc\\b.h",
// JSON:             "{{.*}}inc\\d.h"
// JSON:         ]
// JSON:     }
// JSON: }

// A directory (trailing separator) gets <source name>.json.
// RUN: rm -rf %t.dir && mkdir -p %t.dir
// RUN: %clang_cl /c /sourceDependencies %t.dir/ /I %S/Inputs/cl-deps/inc -o %t.obj -- %S/Inputs/cl-deps/main.c
// RUN: FileCheck --input-file=%t.dir/main.c.json --check-prefix=JSON %s
