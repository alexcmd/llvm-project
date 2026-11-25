// CL-DX-EXPLICIT-INST-UNDEFINED: cl 19.44 accepts an explicit instantiation
// definition of a function template that has no definition in the translation
// unit (MiniEngine IndexOptimizePostTransform.h: template void
// OptimizeFaces<uint16_t, uint16_t>(...); the body is in another .cpp), silent
// at /Wall and with /permissive-, and reports C4661 (level 1) for an undefined
// member function or static data member of a class template. Nothing is
// instantiated; a later definition is still instantiated at the end of the
// translation unit.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -emit-llvm -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=CL %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /wd4661 -fsyntax-only -- %s 2>&1 | count 0
// CL: warning: explicit instantiation of undefined member function 'f' of class template 'X<int>' instantiates nothing; Microsoft extension [-Wmicrosoft-explicit-instantiation-undefined-member]
// CL: warning: explicit instantiation of undefined static data member 's' of class template 'X<int>' instantiates nothing; Microsoft extension [-Wmicrosoft-explicit-instantiation-undefined-member]
// CL-NOT: ): warning:

template <typename S, typename D> void Opt(const S *in, unsigned long long n, D *out); // ms-note {{explicit instantiation refers here}} \
                                                                                     // std-note {{explicit instantiation refers here}}
template void Opt<unsigned short, unsigned short>(const unsigned short *, unsigned long long, unsigned short *);
// ms-warning@-1 {{explicit instantiation of undefined function template 'Opt' instantiates nothing; Microsoft extension}}
// std-error@-2 {{explicit instantiation of undefined function template 'Opt'}}

template <class T> struct X {
  void f();      // ms-note {{explicit instantiation refers here}} std-note {{explicit instantiation refers here}}
  static int s;  // ms-note {{explicit instantiation refers here}} std-note {{explicit instantiation refers here}}
};
template void X<int>::f(); // ms-warning {{explicit instantiation of undefined member function 'f' of class template 'X<int>' instantiates nothing}} \
                           // std-error {{explicit instantiation of undefined member function 'f' of class template 'X<int>'}}
template int X<int>::s;    // ms-warning {{explicit instantiation of undefined static data member 's' of class template 'X<int>' instantiates nothing}} \
                           // std-error {{explicit instantiation of undefined static data member 's' of class template 'X<int>'}}

// Defined after the explicit instantiation: instantiated (both modes).
template <class T> T later(T t);
template int later<int>(int);
template <class T> T later(T t) { return t; }

// A call to the undefined instantiation is an external reference.
void use(unsigned short *p) { Opt(p, 1, p); X<int>().f(); }

// IR-DAG: define weak_odr dso_local noundef i32 @"??$later@H@@YAHH@Z"
// IR-DAG: declare dso_local void @"??$Opt@GG@@YAXPEBG_KPEAG@Z"
// IR-DAG: declare dso_local void @"?f@?$X@H@@QEAAXXZ"
// IR-NOT: define {{.*}}Opt@
