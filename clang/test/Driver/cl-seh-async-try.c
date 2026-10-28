// Note: %s must be preceded by --, otherwise it may be interpreted as a
// command-line option, e.g. on Mac where %s is commonly under /Users.

// clang-cl enables -fms-seh-async-try by default on x86-64 under every /EH
// mode except /EHa (which already implies it).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### -- %s 2>&1 | FileCheck -check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c /EHsc -### -- %s 2>&1 | FileCheck -check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c /EHs-c- -### -- %s 2>&1 | FileCheck -check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c /kernel -### -- %s 2>&1 | FileCheck -check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -fno-ms-seh-async-try -fms-seh-async-try -### -- %s 2>&1 | FileCheck -check-prefix=ON %s
// ON: "-fms-seh-async-try"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -fno-ms-seh-async-try -### -- %s 2>&1 | FileCheck -check-prefix=OFF %s
// RUN: %clang_cl --target=i686-pc-windows-msvc /c -### -- %s 2>&1 | FileCheck -check-prefix=OFF %s
// RUN: %clang_cl --target=aarch64-pc-windows-msvc /c -### -- %s 2>&1 | FileCheck -check-prefix=OFF %s
// OFF-NOT: "-fms-seh-async-try"
// OFF-NOT: warning: argument unused

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c /EHa -### -- %s 2>&1 | FileCheck -check-prefix=EHA %s
// EHA: "-fasync-exceptions"
// EHA-NOT: "-fms-seh-async-try"

// Explicit opt-in on other MSVC targets.
// RUN: %clang_cl --target=aarch64-pc-windows-msvc /c -fms-seh-async-try -### -- %s 2>&1 | FileCheck -check-prefix=ON %s

// Outside clang-cl it is opt-in.
// RUN: %clang --target=x86_64-pc-windows-msvc -c -### %s 2>&1 | FileCheck -check-prefix=OFF %s
// RUN: %clang --target=x86_64-pc-windows-msvc -c -fms-seh-async-try -### %s 2>&1 | FileCheck -check-prefix=ON %s
// RUN: %clang --target=x86_64-pc-windows-msvc -c -fms-seh-async-try -fno-ms-seh-async-try -### %s 2>&1 | FileCheck -check-prefix=OFF %s
