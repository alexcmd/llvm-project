// RUN: %clang_cc1 -triple x86_64-windows -fms-extensions -Wno-implicit-function-declaration -emit-llvm %s -o - | FileCheck %s

// CHECK: %[[src:[0-9-]+]] = call ptr @llvm.localaddress()
// CHECK-NEXT: %cleanup.dest = load i32, ptr %cleanup.dest.slot, align 4
// CHECK-NEXT: %[[src2:[0-9-]+]] = icmp ne i32 %cleanup.dest, 0
// x64 (cl): an abnormal exit runs the __finally through _local_unwind (the
// marked call), the normal one calls it directly.
// CHECK-NEXT: br i1 %[[src2]], label %seh.finally.local_unwind, label %seh.finally.direct
// CHECK: seh.finally.local_unwind:
// CHECK-NEXT: notail call void @"?fin$0@0@seh_abnormal_exits@@"(i8 noundef 1, ptr noundef %[[src]]) #[[LU:[0-9]+]]
// CHECK: seh.finally.direct:
// CHECK-NEXT: call void @"?fin$0@0@seh_abnormal_exits@@"(i8 noundef 0, ptr noundef %[[src]])
// CHECK: attributes #[[LU]] = { noinline "msvc-seh-local-unwind" }

void seh_abnormal_exits(int *Counter) {
  for (int i = 0; i < 5; i++) {
    __try {
      if (i == 0)
        continue;   // abnormal termination
      else if (i == 1)
        goto t10;   // abnormal termination
      else if (i == 2)
        __leave;  // normal execution
      else if (i == 4)
        return;  // abnormal termination
    }
    __finally {
      if (AbnormalTermination()) {
        *Counter += 1;
      }
    }
  t10:;
  }
  return; // *Counter == 3
}

