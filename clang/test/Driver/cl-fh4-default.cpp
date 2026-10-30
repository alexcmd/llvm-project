// Default EH frame handler of clang-cl matches cl.exe: __CxxFrameHandler4 on
// x64, __CxxFrameHandler3 on x86 and arm64; /d2FH4- selects FH3 on x64.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -- %s 2>&1 | FileCheck --check-prefix=X64 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /d2FH4 /c -### -- %s 2>&1 | FileCheck --check-prefix=X64 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /d2FH4- /c -### -- %s 2>&1 | FileCheck --check-prefix=FH3 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /d2FH4- /d2FH4 /c -### -- %s 2>&1 | FileCheck --check-prefix=X64 %s
// RUN: %clang_cl --target=i686-pc-windows-msvc /c -### -- %s 2>&1 | FileCheck --check-prefix=FH3 %s
// RUN: %clang_cl --target=aarch64-pc-windows-msvc /c -### -- %s 2>&1 | FileCheck --check-prefix=FH3 %s
// RUN: %clang --target=x86_64-pc-windows-msvc -c -### %s 2>&1 | FileCheck --check-prefix=FH3 %s

// X64: "-fms-cxx-eh4"
// FH3-NOT: "-fms-cxx-eh4"
