// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -std=c++20 -emit-llvm -o - %s | FileCheck %s
// RUN: not %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -emit-llvm -o %t %s 2>&1 | FileCheck --check-prefix=ID %s

// ID: definition of builtin function '__builtin_ctzll'
// ID: definition of builtin function '__builtin_clzll'
int counter;
// CHECK: define {{.*}}i32 @__builtin_ctzll(
// CHECK: store i32 {{.*}}, ptr @"?counter@@3HA"
int __builtin_ctzll(unsigned long long x) { ++counter; return x ? 123 : 456; }
// CHECK: define {{.*}}i32 @__builtin_clzll(
int __builtin_clzll(unsigned long long x) { return x ? 789 : 321; }
// CHECK-LABEL: define {{.*}}@"?probe@@
// CHECK: call {{.*}}i32 @__builtin_ctzll(
// CHECK: call {{.*}}i32 @__builtin_clzll(
// CHECK-NOT: llvm.cttz
// CHECK-NOT: llvm.ctlz
int probe(unsigned long long x) { return __builtin_ctzll(x) + __builtin_clzll(x); }
