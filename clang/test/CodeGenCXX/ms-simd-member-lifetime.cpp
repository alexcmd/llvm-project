// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -O1 -disable-llvm-passes -emit-llvm -o - %s | FileCheck %s
typedef float __m128 __attribute__((vector_size(16)));
__m128 make();
void side();
// CHECK-LABEL: define {{.*}}@"?retain@@
// CHECK: call {{.*}}@"?make@@
// CHECK-NOT: lifetime.end
// CHECK: call void @"?side@@
// CHECK: load float
// CHECK: lifetime.end
float retain() {
  const float (&r)[4] = make().m128_f32;
  side();
  return r[2];
}
