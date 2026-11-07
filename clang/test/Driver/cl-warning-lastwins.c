// openrt UE-C4244-SHORTEN, UE-WE-LASTWINS, UE-WD5054, UE-PRAGMA-PACK-C4103:
// cl warning ids map as cl numbers them, and for one id the last of /we /wd
// /w<l> /wo wins; /w<l><n> above the final /W level is off (cl 14.51).

// C4244 = 64-to-32 from a type other than size_t, C4267 = from size_t.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W4 /we4244 /wd4267 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=MAP %s
// MAP: "-Werror=shorten-int64-to-32"
// MAP-SAME: "-Wno-shorten-size-t"

// Both are on from /W3 (cl level 3), off below.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W3 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=W3 %s
// W3-DAG: "-Wshorten-int64-to-32"
// W3-DAG: "-Wshorten-size-t"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W2 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=W2 %s
// W2-DAG: "-Wno-shorten-int64-to-32"
// W2-DAG: "-Wno-shorten-size-t"

// /we then /w4: a warning again.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W4 /we4244 /w44244 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=LAST %s
// LAST: "-Werror=shorten-int64-to-32" "-Wno-error=shorten-int64-to-32" "-Wshorten-int64-to-32"
// ... but not under /WX.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W4 /WX /we4244 /w44244 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=LASTWX %s
// LASTWX: "-Werror=shorten-int64-to-32" "-Wshorten-int64-to-32"
// A level-4 /w4 under level-3 /W3 is off, whatever the order.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /w44244 /W3 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=LEVEL %s
// LEVEL: "-Wno-shorten-int64-to-32"
// LEVEL-NOT: "-Wshorten-int64-to-32"

// C5054 covers named and anonymous enum arithmetic and enum comparison.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /W4 /wd5054 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=WD5054 %s
// WD5054: "-Wno-deprecated-enum-enum-conversion" "-Wno-deprecated-anon-enum-enum-conversion" "-Wno-deprecated-enum-compare"

// C4103 is the pragma pack change across an #include.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /wd4103 -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=WD4103 %s
// WD4103: "-Wno-pragma-pack"
