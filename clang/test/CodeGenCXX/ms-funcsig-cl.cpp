// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fno-ms-clang-identity -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -emit-llvm -o - %s | FileCheck --check-prefix=ID %s

// openrt UE-FUNCSIG-CL: without the clang identity macros, __FUNCSIG__ reads
// as cl 14.51 writes it (UE's generated type names parse the text between
// the first '<' and the last '>'). Expected strings measured with cl.

namespace N { struct S {}; class C {}; union U { int a; }; enum E { e0 }; enum class EC { a };
template <class T> struct TT {}; }
enum E2 { f0, f1 }; int g;
template <typename... T> const char *G() { return __FUNCSIG__; }
template <int I, bool B, char C> const char *V() { return __FUNCSIG__; }
template <E2 X, int *P, long long L> const char *W() { return __FUNCSIG__; }
template <typename T> struct Outer {
  template <class U> const char *mf(U &, const U *) const { return __FUNCSIG__; }
  static const char *sf() { return __FUNCSIG__; }
};
struct A { const char *m(N::S, const N::S &, N::S *) { return __FUNCSIG__; } };
const char *plain(int, const char *, double &) { return __FUNCSIG__; }

// CHECK-DAG: c"const char *__cdecl G<struct N::S,class N::C,union N::U,enum N::E,enum N::EC>(void)\00"
// CHECK-DAG: c"const char *__cdecl G<int*,const int*,int*const ,int&,int&&,const int&>(void)\00"
// CHECK-DAG: c"const char *__cdecl G<struct N::TT<struct N::TT<int> >,unsigned __int64,long double>(void)\00"
// CHECK-DAG: c"const char *__cdecl G<void(__cdecl *)(int,char),int[4],intN::S::* >(void)\00"
// CHECK-DAG: c"const char *__cdecl V<0xfffffffffffffffd,true,0x78>(void)\00"
// CHECK-DAG: c"const char *__cdecl W<f1,&g,0xfffffffffffffffb>(void)\00"
// CHECK-DAG: c"const char *__cdecl Outer<struct N::S>::mf<int>(int &,const int *) const\00"
// CHECK-DAG: c"const char *__cdecl Outer<int>::sf(void)\00"
// CHECK-DAG: c"const char *__cdecl A::m(struct N::S,const struct N::S &,struct N::S *)\00"
// CHECK-DAG: c"const char *__cdecl plain(int,const char *,double &)\00"
// CHECK-DAG: c"const char *__cdecl G<>(void)\00"
// ID-DAG: c"const char *__cdecl G(void) [T = <N::S, N::C, N::U, N::E, N::EC>]\00"

// noexcept, then const/volatile/ref-qualifier; lambdas (measured, cl 14.51).
const char *nx() noexcept { return __FUNCSIG__; }
struct Q {
  const char *b() const noexcept { return __FUNCSIG__; }
  const char *c() volatile { return __FUNCSIG__; }
  const char *d() & { return __FUNCSIG__; }
  const char *m2() { auto a = [] { return 1; }; auto l = [this](int) -> const char * { return __FUNCSIG__; }; (void)a; return l(1); }
};
const char *f2() { auto a = [] { return __FUNCSIG__; }; auto b = [](int, double &) { return __FUNCSIG__; }; double d; a(); return b(1, d); }
const char *g1() { auto l = [](auto) { return __FUNCSIG__; }; return l(3); }
const char *n1() { auto o = [] { auto in = [] { return __FUNCSIG__; }; return in(); }; return o(); }
auto gl = [] { return __FUNCSIG__; };
const char *mn() { auto l = []() noexcept { return __FUNCSIG__; }; auto m = [](int) mutable { return __FUNCSIG__; }; l(); return m(1); }
// CHECK-DAG: c"const char *__cdecl nx(void) noexcept\00"
// CHECK-DAG: c"const char *__cdecl Q::b(void) noexcept const\00"
// CHECK-DAG: c"const char *__cdecl Q::c(void) volatile\00"
// CHECK-DAG: c"const char *__cdecl Q::d(void) &\00"
// CHECK-DAG: c"const char *__cdecl Q::m2::<lambda_2>::operator ()(int) const\00"
// CHECK-DAG: c"auto __cdecl f2::<lambda_1>::operator ()(void) const\00"
// CHECK-DAG: c"auto __cdecl f2::<lambda_2>::operator ()(int,double &) const\00"
// CHECK-DAG: c"auto __cdecl g1::<lambda_1>::operator ()<int>(int) const\00"
// CHECK-DAG: c"auto __cdecl n1::<lambda_1>::()::<lambda_1>::operator ()(void) const\00"
// CHECK-DAG: c"auto __cdecl <lambda>@gl::operator ()(void) const\00"
// CHECK-DAG: c"auto __cdecl mn::<lambda_1>::operator ()(void) noexcept const\00"
// CHECK-DAG: c"auto __cdecl mn::<lambda_2>::operator ()(int)\00"
void use2() { Q q; q.b(); q.c(); q.d(); q.m2(); nx(); f2(); g1(); n1(); gl(); mn(); }
void use() {
  int i = 0; double d = 0;
  G<N::S, N::C, N::U, N::E, N::EC>();
  G<int *, const int *, int *const, int &, int &&, const int &>();
  G<N::TT<N::TT<int>>, unsigned long long, long double>();
  G<void (*)(int, char), int[4], int(N::S::*)>();
  V<-3, true, 'x'>();
  W<f1, &g, -5>();
  Outer<N::S>().mf(i, &i);
  Outer<int>::sf();
  A().m({}, {}, nullptr);
  plain(1, "", d);
  G<>();
}
