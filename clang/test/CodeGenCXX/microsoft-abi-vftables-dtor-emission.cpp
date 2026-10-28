// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -emit-llvm -o - %s | FileCheck %s --check-prefix=NEG

// Like MSVC, a TU defining a virtual destructor out of line emits the class's
// vftables, deleting destructor and RTTI, even if the destructor body is
// trivial and never stores the vfptr.

struct A {
  virtual ~A();
  virtual void f();
};
A::~A() {}

// CHECK-DAG: @{{[0-9]+}} = private constant { [3 x ptr] } { [3 x ptr] [ptr @"??_R4A@@6B@", ptr @"??_EA@@UEAAPEAXI@Z", ptr @"?f@A@@UEAAXXZ"] }, comdat($"??_7A@@6B@")
// CHECK-DAG: @"??_7A@@6B@" = alias ptr, getelementptr inbounds ({ [3 x ptr] }, ptr @{{[0-9]+}}, i32 0, i32 0, i32 1)
// CHECK-DAG: define linkonce_odr dso_local noundef ptr @"??_GA@@UEAAPEAXI@Z"(

// All vfptrs of the class get their vftable.
struct B1 {
  virtual ~B1();
};
struct B2 {
  virtual void g();
};
struct C : B1, B2 {
  ~C();
};
C::~C() {}

// CHECK-DAG: @"??_7C@@6BB1@@@" = alias ptr,
// CHECK-DAG: @"??_7C@@6BB2@@@" = alias ptr,
// CHECK-DAG: define linkonce_odr dso_local noundef ptr @"??_GC@@UEAAPEAXI@Z"(

// Not for TUs that only define another virtual method or a non-virtual dtor.
struct E {
  virtual ~E();
  virtual void f();
};
void E::f() {}

struct F {
  ~F();
  virtual void f();
};
F::~F() {}

// Not for inline virtual dtors, which are still emitted on demand.
struct G {
  virtual ~G() {}
  virtual void f();
};
void useG(G *g) { g->f(); }

// NEG-NOT: ??_7E@@
// NEG-NOT: ??_7F@@
// NEG-NOT: ??_7G@@
