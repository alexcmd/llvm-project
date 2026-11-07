// openrt UE-W0-WE: /W0 and /w turn every warning off, but cl 14.51 still
// reports an id named by /we as an error (C4244 under /W0 /we4244, either
// order), and the /Zc:strictStrings error. clang-cl then uses
// -Wno-everything instead of -w, and the /we group stays an error.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W0 /we4244 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=KEEP %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /we4244 /w -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=KEEP %s
// KEEP-NOT: "-w"
// KEEP: "-Wno-everything"
// KEEP-SAME: "-Werror=shorten-int64-to-32"
// KEEP-NOT: "-w"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /w /Zc:strictStrings -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=STRICT %s
// STRICT: "-Wno-everything"
// STRICT-SAME: "-Werror=writable-strings"

// Without /we it stays -w; an unmapped /we id changes nothing.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W0 /we9999 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=PLAIN %s
// PLAIN-NOT: "-Wno-everything"
// PLAIN: "-w"
