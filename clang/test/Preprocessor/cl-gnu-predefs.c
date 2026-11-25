// CL-DX-GNU-PREDEFS: cl 19.44 defines none of the GNU-style macros below. In
// cl mode (clang identity hidden) clang-cl leaves out those that nothing in
// the resource headers tests; the ones the headers read stay.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /EP -- %s | FileCheck --check-prefix=CL %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /EP /clang:-fms-clang-identity -- %s | FileCheck --check-prefix=ID %s

#ifdef __amd64
amd64_defined
#endif
#ifdef __SSE__
sse_defined
#endif
#ifdef __SIZEOF_INT__
sizeof_int_defined
#endif
#ifdef __INT_FAST8_TYPE__
int_fast8_defined
#endif
#ifdef __PIC__
pic_defined
#endif
#ifdef __USER_LABEL_PREFIX__
user_label_prefix_defined
#endif
// CL-NOT: _defined
// CL: kept: x86_64 1 sse2 1 size_type long long unsigned int flt_mant 24
// ID: amd64_defined
// ID: sse_defined
// ID: sizeof_int_defined
// ID: int_fast8_defined
// ID: pic_defined
// ID: user_label_prefix_defined
kept: x86_64 __x86_64__ sse2 __SSE2__ size_type __SIZE_TYPE__ flt_mant __FLT_MANT_DIG__
