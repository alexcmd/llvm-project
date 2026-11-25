// CL-DX-PP-UNDEF-CALL: cl 19.44 takes "#if undefined(X)" (and "#if foo(1)"
// with foo not a macro) as the identifier alone, 0, and ignores the rest of
// the line with C4067 (level 1). The clang driver keeps the error.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /EP -- %s 2>%t.err | FileCheck --check-prefix=OUT %s
// RUN: FileCheck --check-prefix=WARN --input-file=%t.err %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /Zs /wd4067 -- %s 2>&1 >/dev/null | count 0
// RUN: not %clang --target=x86_64-pc-linux -E %s 2>&1 | FileCheck --check-prefix=ERR %s

#if undefined(X)
int yes;
#else
int no;
#endif
#if 1 && foo(1)
int yes2;
#endif
// OUT-NOT: yes
// OUT: int no;
// OUT-NOT: yes
// WARN: (10,5): warning: function-like macro 'undefined' is not defined; it evaluates to 0 and the rest of the directive is ignored; Microsoft extension [-Wmicrosoft-pp-undefined-call]
// WARN: (15,10): warning: function-like macro 'foo' is not defined
// ERR: error: function-like macro 'undefined' is not defined
