// RUN: %clang_cc1 -emit-llvm -o - -triple=x86_64-pc-windows-msvc -std=c++17 %s -fcxx-exceptions -fexceptions -fms-extensions | FileCheck %s

// The catchable type's copy function must be the copy constructor, as with
// MSVC, even when overload resolution for a non-const lvalue would pick a
// forwarding constructor template (std::_With_nested_v2, GH#191462).

struct Fwd {
  Fwd();
  Fwd(const Fwd &);
  template <class T> explicit Fwd(T &&);
};
// A class whose only copy constructor takes a non-const reference keeps it.
struct NonConstCopy {
  NonConstCopy();
  NonConstCopy(NonConstCopy &);
};
struct Base { Base(); Base(const Base &); template <class T> Base(T &&); };
struct Derived : Base {};
// Implicit trivial copy constructor: no copy function (MSVC: _CT??_R0?AUImpl@@@81).
struct Impl { Impl(); ~Impl(); template <class T> explicit Impl(T &&); };

void f(int i) {
  if (i == 0) throw Fwd();
  if (i == 1) throw NonConstCopy();
  if (i == 2) throw Derived();
  Impl x;
  throw x;
}

// CHECK-DAG: @"_CT??_R0?AUFwd@@@8??0Fwd@@QEAA@AEBU0@@Z1" = {{.*}}@"??0Fwd@@QEAA@AEBU0@@Z"
// CHECK-DAG: @"_CT??_R0?AUNonConstCopy@@@8??0NonConstCopy@@QEAA@AEAU0@@Z1" = {{.*}}@"??0NonConstCopy@@QEAA@AEAU0@@Z"
// CHECK-DAG: @"_CT??_R0?AUDerived@@@8??0Derived@@QEAA@AEBU0@@Z1" = {{.*}}@"??0Derived@@QEAA@AEBU0@@Z"
// CHECK-DAG: @"_CT??_R0?AUBase@@@8??0Base@@QEAA@AEBU0@@Z1" = {{.*}}@"??0Base@@QEAA@AEBU0@@Z"
// CHECK-DAG: @"_CT??_R0?AUImpl@@@81" = {{.*}}, i32 1, i32 0 }, section ".xdata"
// CHECK-NOT: ??$?0
