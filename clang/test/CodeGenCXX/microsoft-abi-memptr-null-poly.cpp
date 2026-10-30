// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -emit-llvm -o - %s | FileCheck %s
// Null data member pointers in single-field models are -1, polymorphic or not, as
// cl.exe (task 21, PMF-NULL-DATA-POLY). Function-template arguments are always 0,
// class-template arguments -1 (single field) or {0,-1} (virtual base), as cl.exe mangles.
struct __single_inheritance SP { int a; virtual void f() {} };
struct __single_inheritance SN { int a; };
struct VB { int b; };
struct VD : virtual VB { int c; virtual void g() {} };
int SP::*psp = nullptr;
int SN::*psn = nullptr;
int VD::*pvd = nullptr;
// CHECK-DAG: @"?psp{{[^=]*}}= {{.*}}global i32 -1
// CHECK-DAG: @"?psn{{[^=]*}}= {{.*}}global i32 -1
// CHECK-DAG: @"?pvd{{[^=]*}}= {{.*}}global { i32, i32 } { i32 0, i32 -1 }

template <class T, int T::*M> void fF() {}
template <class T, int T::*M> struct CF {};
void fF_SP() { fF<SP, nullptr>(); }
void fF_SN() { fF<SN, nullptr>(); }
void cF_SP(CF<SP, nullptr> *) {}
void cF_SN(CF<SN, nullptr> *) {}
void cF_VD(CF<VD, nullptr> *) {}
// CHECK-DAG: define {{.*}}@"??$fF@USP@@$0A@@@YAXXZ"
// CHECK-DAG: define {{.*}}@"??$fF@USN@@$0A@@@YAXXZ"
// CHECK-DAG: define {{.*}}@"?cF_SP@@YAXPEAU?$CF@USP@@$0?0@@@Z"
// CHECK-DAG: define {{.*}}@"?cF_SN@@YAXPEAU?$CF@USN@@$0?0@@@Z"
// CHECK-DAG: define {{.*}}@"?cF_VD@@YAXPEAU?$CF@UVD@@$FA@?0@@@Z"
