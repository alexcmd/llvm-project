// The fork identity appears in the clang-cl version line: openrt marker plus
// the commit, never a Microsoft product name.

// RUN: %clang_cl /clang:--version | FileCheck %s
// RUN: %clang_cl /clang:-fms-clang-identity --version | FileCheck %s
// RUN: %clang --driver-mode=cl -fms-clang-identity --version | FileCheck %s
// RUN: %clang --version | FileCheck %s
// CHECK: clang version {{.*}} (openrt {{[0-9a-f]+}})
// CHECK-NOT: Microsoft

// CL-IDENTITY: cl.exe has no --version: with the clang identity hidden (cl
// mode default) a bare --version is an unknown option (warning, then no
// input files), so CMake's generic ASM probe does not see "clang version".
// RUN: not %clang_cl --version 2>&1 | FileCheck %s --check-prefix=UNK
// RUN: not %clang_cl /nologo --version 2>&1 | FileCheck %s --check-prefix=UNK
// UNK-NOT: clang version
// UNK: warning: unknown argument ignored in clang-cl: '--version'
// UNK: error: no input files

// With a source file it still compiles, as cl does.
// RUN: %clang_cl --version /Zs -- %s 2>&1 | FileCheck %s --check-prefix=SRC
// SRC-NOT: clang version
// SRC: warning: unknown argument ignored in clang-cl: '--version'
