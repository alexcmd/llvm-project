// CL-DX-FILE-NAME: cl 19.44 has no __FILE_NAME__, __BASE_FILE__ or
// __INCLUDE_LEVEL__; they stay identifiers in cl mode (clang identity hidden).

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /EP -- %s | FileCheck --check-prefix=CL %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /EP /clang:-fms-clang-identity -- %s | FileCheck --check-prefix=ID %s

A __FILE_NAME__ B __BASE_FILE__ C __INCLUDE_LEVEL__
// CL: A __FILE_NAME__ B __BASE_FILE__ C __INCLUDE_LEVEL__
// ID: A "cl-file-name-macros.c" B "{{.*}}cl-file-name-macros.c" C 0
#ifdef __FILE_NAME__
file_name_defined
#endif
// CL-NOT: file_name_defined
// ID: file_name_defined
