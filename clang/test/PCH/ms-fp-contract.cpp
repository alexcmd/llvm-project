// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -ffp-contract=fast -include %s -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -ffp-contract=fast -emit-pch -o %t %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -ffp-contract=fast -include-pch %t -emit-llvm -o - %s | FileCheck %s

#ifndef HEADER
#define HEADER
#pragma float_control(push)
#pragma fp_contract(off)
template <class T> T no_contract(T a, T b, T c) { return a * b + c; }
#else
extern "C" float use_template(float a, float b, float c) {
  return no_contract(a, b, c);
}
// CHECK-LABEL: define {{.*}} @"??$no_contract@M@@
// CHECK: fmul float
// CHECK: fadd float
// CHECK-LABEL: define {{.*}} @after_pch(
// CHECK: fmul float
// CHECK: fadd float
extern "C" float after_pch(float a, float b, float c) { return a * b + c; }
#pragma float_control(pop)
// CHECK-LABEL: define {{.*}} @restored(
// CHECK: fmul contract float
// CHECK: fadd contract float
extern "C" float restored(float a, float b, float c) { return a * b + c; }
#endif
