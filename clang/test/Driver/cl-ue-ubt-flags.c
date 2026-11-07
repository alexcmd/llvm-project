// openrt UE-DX-IMPLVIRT, UE-ZC-STRICTSTRINGS-OFF: flags
// UnrealBuildTool passes on every compile; cl 14.51 is silent on them, also
// under /WX.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /WX /c -### \
// RUN:   /dximplicitlyinstantiatevirtualfunction- /dxanything /permissive- \
// RUN:   /Zc:strictStrings- -- %s 2>&1 | FileCheck --check-prefix=SILENT %s
// SILENT-NOT: D9002
// SILENT-NOT: argument unused
// SILENT-NOT: error
// SILENT-NOT: "-Werror=writable-strings"
// SILENT: "-cc1"
// SILENT-NOT: "-Werror=writable-strings"

// A bare /dx is D8004.
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /c -### /dx -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=DXBARE %s
// DXBARE: D8004 : '/dx'

// /Zc:strictStrings (and /permissive-) make the string literal to char *
// conversion an error; the last of them and their '-' forms wins.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /Zc:strictStrings -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=STRICT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /permissive- -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=STRICT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /Zc:strictStrings- /permissive- -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=STRICT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /permissive /Zc:strictStrings -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=STRICT %s
// STRICT: "-Werror=writable-strings"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /Zc:strictStrings /permissive -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NOSTRICT %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /Zc:strictStrings /Zc:strictStrings- -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NOSTRICT %s
// NOSTRICT-NOT: "-Werror=writable-strings"
