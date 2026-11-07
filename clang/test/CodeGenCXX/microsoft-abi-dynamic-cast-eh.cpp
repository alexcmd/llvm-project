// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fcxx-exceptions -fexceptions \
// RUN:   -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-windows-msvc -fcxx-exceptions -fexceptions \
// RUN:   -emit-llvm -o - %s | FileCheck %s

// openrt DYNCAST-VOID-UNWIND (cl 14.51 /EHsc): the RTTI runtime calls throw
// std::__non_rtti_object for an object without RTTI data; cl lets a try
// around the expression catch it. Each call is an invoke inside a try.

namespace std { class type_info; }
struct B { virtual ~B(); };
struct D : B {};
void sink(const void *);

// CHECK-LABEL: define {{.*}}@"?cast_void@@
// CHECK: invoke {{.*}}ptr @__RTCastToVoid(
void cast_void(B *p) { try { sink(dynamic_cast<void *>(p)); } catch (...) {} }

// CHECK-LABEL: define {{.*}}@"?cast_down@@
// CHECK: invoke {{.*}}ptr @__RTDynamicCast(
void cast_down(B *p) { try { sink(dynamic_cast<D *>(p)); } catch (...) {} }

// CHECK-LABEL: define {{.*}}@"?type_id@@
// CHECK: invoke {{.*}}ptr @__RTtypeid(
void type_id(B *p) { try { sink(&typeid(*p)); } catch (...) {} }
