// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -O1 -disable-llvm-passes -emit-llvm -verify -o - %s | FileCheck %s
// expected-no-diagnostics

// openrt UE-PRAGMA-OPTIMIZE-CLASS: cl 14.51 takes #pragma optimize in a class
// body silently, as a change of the state at that point; a member function
// defined in the class is compiled with the state at the end of the class
// (measured, /FA): here Construct stays optimized, after() is not.

struct Widget {
  __pragma(optimize("", off))
  int Construct(int v) { return v * 2; }
  __pragma(optimize("", on))
};
struct W2 {
#pragma optimize("", off)
  int a;
};
int after(int v) { return v + 1; }
#pragma optimize("", on)
int run() { Widget w; return w.Construct(3) + after(1); }

// CHECK: define {{.*}}@"?after@@YAHH@Z"({{.*}}) #[[OFF:[0-9]+]]
// CHECK: define {{.*}}@"?Construct@Widget@@QEAAHH@Z"({{.*}}) #[[ON:[0-9]+]]
// CHECK: attributes #[[OFF]] = {{{.*}}optnone
// CHECK-NOT: attributes #[[ON]] = {{{.*}}optnone
