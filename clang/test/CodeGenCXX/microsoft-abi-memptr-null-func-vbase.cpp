// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -emit-llvm -o - %s | FileCheck %s
// A null function member pointer of a class with a virtual base is 16 zero bytes
// (cl.exe, task 21 PMF-NULL-VBASE). Null tests use only the function pointer field.
struct VB { int b; };
struct VD : virtual VB { int c; virtual void g() {} };
void (VD::*fnull)() = nullptr;
// CHECK-DAG: @"?fnull{{[^=]*}}= {{.*}}global {{.*}}zeroinitializer
bool isnull(void (VD::*p)()) { return p == nullptr; }
// CHECK-LABEL: define {{.*}}@"?isnull@@
// CHECK-NOT: extractvalue
// CHECK: icmp
