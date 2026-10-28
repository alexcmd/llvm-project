// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-extensions -fms-seh-async-try -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-extensions -emit-llvm -o - %s | FileCheck %s --check-prefix=SYNC

// -fms-seh-async-try: an SEH __try catches a hardware exception raised by any
// instruction of its body (cl behavior under /EHsc), not only by calls. The
// body is bracketed by llvm.seh.try.begin/end like under -fasync-exceptions,
// but only the function with the __try is affected and there is no
// "eh-asynch" module flag.

int fault_except(int *p) {
  int r = 0;
  __try {
    r = *p;
  } __except (1) {
    r = -1;
  }
  return r;
}

// CHECK-LABEL: define dso_local i32 @fault_except(ptr noundef %p)
// CHECK-SAME: #[[ATTR:[0-9]+]] personality ptr @__C_specific_handler
// CHECK: invoke void @llvm.seh.try.begin()
// CHECK-NEXT: to label %[[CONT:[^ ]+]] unwind label %[[DISPATCH:[^ ]+]]
// CHECK: [[CONT]]:
// CHECK: load volatile ptr, ptr %p.addr
// CHECK: load volatile i32, ptr
// CHECK: invoke void @llvm.seh.try.end()
// CHECK-NEXT: to label %{{[^ ]+}} unwind label %[[DISPATCH]]
// CHECK: [[DISPATCH]]:
// CHECK-NEXT: catchswitch within none [label %__except] unwind to caller
// CHECK: catchpad within %{{[^ ]+}} [ptr null]
// CHECK: store i32 -1, ptr %r

// Without the flag the __except is dropped: the body has no call.
// SYNC-LABEL: define dso_local i32 @fault_except(ptr noundef %p)
// SYNC-NOT: personality
// SYNC-NOT: catchswitch
// SYNC: ret i32

int filter(void);
int fault_filter(int *p) {
  int r = 0;
  __try {
    r = *p;
  } __except (filter()) {
    r = -1;
  }
  return r;
}

// CHECK-LABEL: define dso_local i32 @fault_filter(ptr noundef %p)
// CHECK-SAME: #[[ATTR]] personality ptr @__C_specific_handler
// CHECK: invoke void @llvm.seh.try.begin()
// CHECK: load volatile i32, ptr
// CHECK: invoke void @llvm.seh.try.end()
// CHECK: catchpad within %{{[^ ]+}} [ptr @"?filt$0@0@fault_filter@@"]

// The outlined filter itself has no __try.
// CHECK: define internal i32 @"?filt$0@0@fault_filter@@"(ptr noundef %exception_pointers, ptr noundef %frame_pointer) #[[HELPER:[0-9]+]]

void use(int);
void fault_finally(int *p) {
  __try {
    *p = 1;
  } __finally {
    use(2);
  }
}

// CHECK-LABEL: define dso_local void @fault_finally(ptr noundef %p)
// CHECK-SAME: #[[ATTR]] personality ptr @__C_specific_handler
// CHECK: invoke void @llvm.seh.try.begin()
// CHECK-NEXT: to label %{{[^ ]+}} unwind label %[[CLEANUP:[^ ]+]]
// CHECK: store volatile i32 1, ptr
// CHECK: invoke void @llvm.seh.try.end()
// CHECK-NEXT: to label %{{[^ ]+}} unwind label %[[CLEANUP]]
// CHECK: call void @"?fin$0@0@fault_finally@@"(i8 noundef 0, ptr noundef %{{[^ ]+}})
// CHECK: [[CLEANUP]]:
// CHECK-NEXT: cleanuppad within none []
// CHECK: call void @"?fin$0@0@fault_finally@@"(i8 noundef 1, ptr noundef %{{[^ ]+}}) [ "funclet"(token %{{[^ ]+}}) ]

int no_try(int *p) { return *p; }

// CHECK-LABEL: define dso_local i32 @no_try(ptr noundef %p)
// CHECK-SAME: #[[NOATTR:[0-9]+]] {
// CHECK-NOT: llvm.seh
// CHECK: ret i32

// CHECK: attributes #[[ATTR]] = { {{.*}}"seh-async-try"{{.*}} }
// CHECK: attributes #[[NOATTR]] = {
// CHECK-NOT: "seh-async-try"
// CHECK-SAME: }
// CHECK-NOT: "eh-asynch"

// SYNC-NOT: "seh-async-try"
