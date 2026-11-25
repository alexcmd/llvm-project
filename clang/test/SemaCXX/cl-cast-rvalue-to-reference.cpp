// CL-DX-CAST-RVALUE-REF: cl 19.44 accepts a class prvalue cast to a non-const
// lvalue reference (MiniEngine: return (OrientedBox&)(xform * obb.m_repr);),
// also with /permissive-, as an lvalue of the temporary (C4238, level 4).
// Microsoft compatibility materializes the temporary; standard mode keeps the
// error.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -DONLY_VALID -emit-llvm -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 -DONLY_VALID -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=W4 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /permissive- -DONLY_VALID -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=W4 %s
// RUN: %clang_cl /nologo --target=x86_64-pc-windows-msvc /W3 -DONLY_VALID -fsyntax-only -- %s 2>&1 >/dev/null | count 0
// RUN: %clang_cl /nologo --target=x86_64-pc-windows-msvc /W4 /wd4238 -DONLY_VALID -fsyntax-only -- %s 2>&1 >/dev/null | count 0
// W4: warning: C-style cast from rvalue of type 'M' to reference type 'Box &' binds a temporary; Microsoft extension [-Wmicrosoft-rvalue-cast-to-reference]
// W4: warning: C-style cast from rvalue of type 'Box' to reference type 'Box &' binds a temporary; Microsoft extension [-Wmicrosoft-rvalue-cast-to-reference]
// W4-NOT: ): warning:

struct M { int v; };
struct Box { M m; Box() {} explicit Box(const M &x) : m(x) {} };
M mul(const M &, const M &);
Box make();

Box f(const M &a, const Box &b) {
  return (Box &)(mul(a, b.m)); // ms-warning {{C-style cast from rvalue of type 'M' to reference type 'Box &' binds a temporary; Microsoft extension}} \
                               // std-error {{C-style cast from rvalue to reference type 'Box &'}}
}

int g() {
  return ((Box &)make()).m.v; // ms-warning {{binds a temporary}} \
                              // std-error {{C-style cast from rvalue to reference type 'Box &'}}
}

#ifndef ONLY_VALID
// Not affected: binding to const T& is standard; non-class prvalues stay errors.
const Box &h() { return (const Box &)make(); } // ms-warning {{returning reference to local temporary object}} \
                                               // std-warning {{returning reference to local temporary object}}
int &k() { return (int &)(1 + 2); } // ms-error {{C-style cast from rvalue to reference type 'int &'}} \
                                    // std-error {{C-style cast from rvalue to reference type 'int &'}}
#endif

// IR-LABEL: define {{.*}}@"?g@@YAHXZ"()
// IR: %[[TMP:.*]] = alloca %struct.Box
// IR: call void @"?make@@YA?AUBox@@XZ"(ptr {{.*}}%[[TMP]])
// IR: getelementptr inbounds nuw %struct.Box, ptr %[[TMP]]
