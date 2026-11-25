// CL-LINK-DEFAULT-LLD: the driver starts its own linker, as cl.exe starts the
// link.exe next to it; -fuse-ld=link asks for the Microsoft one.
// RUN: %clang_cl --target=x86_64-windows-msvc -### -- %s 2>&1 | FileCheck -check-prefix=LLD %s
// RUN: %clang_cl --target=x86_64-windows-msvc -### %s /link /INCREMENTAL:NO 2>&1 | FileCheck -check-prefix=LLD %s
// RUN: %clang_cl --target=x86_64-windows-msvc -fuse-ld=link -### -- %s 2>&1 | FileCheck -check-prefix=MSLINK %s
// LLD: lld-link{{(\.exe)?}}"
//
// LTO needs lld: no error with the default, an error only when another linker
// is named.
// RUN: %clang_cl --target=x86_64-windows-msvc -flto -### -- %s 2>&1 | FileCheck -check-prefix=LTO %s
// RUN: %clang_cl --target=x86_64-windows-msvc -flto=thin -fuse-ld= -### -- %s 2>&1 | FileCheck -check-prefix=LTO %s
// RUN: not %clang_cl --target=x86_64-windows-msvc -flto -fuse-ld=link -### -- %s 2>&1 | FileCheck -check-prefix=LTO-LINK %s
// LTO-NOT: LTO requires -fuse-ld=lld
// LTO: lld-link{{(\.exe)?}}"
// LTO-LINK: error: LTO requires -fuse-ld=lld
// MSLINK: {{["\\/]}}link.exe"
