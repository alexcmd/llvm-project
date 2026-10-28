// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -disable-llvm-passes -emit-llvm -o - %s | FileCheck %s

// [[msvc::forceinline]] after the lambda declarator lands on operator().
int f(int y) {
  auto a = [](int x) noexcept [[msvc::forceinline]] { return x + 1; };
  return a(y);
}

// CHECK: define internal {{.*}}@"??R<lambda_0>@{{[^"]*}}"({{.*}}) #[[ATTR:[0-9]+]]
// CHECK: attributes #[[ATTR]] = { alwaysinline {{.*}}}
