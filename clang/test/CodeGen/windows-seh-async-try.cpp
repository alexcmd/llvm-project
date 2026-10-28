// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-extensions -fcxx-exceptions -fexceptions -fms-seh-async-try -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-extensions -fcxx-exceptions -fexceptions -fms-seh-async-try -emit-llvm -o - %s | FileCheck %s --check-prefix=NOSCOPE

// NOSCOPE-NOT: llvm.seh.scope

// -fms-seh-async-try only affects SEH __try. C++ EH keeps /EHsc semantics:
// no llvm.seh.scope.* around C++ cleanups, catch(...) keeps the
// HT_IsStdDotDot (0x40) adjective so it does not catch SEH exceptions, and
// there is no "eh-asynch" module flag.

struct S { ~S(); };
void may_throw();

void cleanup_only() {
  S s;
  may_throw();
}

// CHECK-LABEL: define dso_local void @"?cleanup_only@@YAXXZ"()
// CHECK-SAME: #[[NOATTR:[0-9]+]] personality ptr @__CxxFrameHandler3
// CHECK-NEXT: entry:
// CHECK-NEXT: %s = alloca %struct.S
// CHECK-NEXT: invoke void @"?may_throw@@YAXXZ"()
// CHECK: ret void
// CHECK: cleanuppad within none []
// CHECK: cleanupret from

void catch_all() {
  try {
    may_throw();
  } catch (...) {
  }
}

// CHECK-LABEL: define dso_local void @"?catch_all@@YAXXZ"()
// CHECK-SAME: #[[NOATTR]] personality ptr @__CxxFrameHandler3
// CHECK-NEXT: entry:
// CHECK-NEXT: invoke void @"?may_throw@@YAXXZ"()
// CHECK: catchpad within %{{[^ ]+}} [ptr null, i32 64, ptr null]

int seh_in_cxx(int *p) {
  int r = 0;
  __try {
    r = *p;
  } __except (1) {
    r = -1;
  }
  return r;
}

// CHECK-LABEL: define dso_local noundef i32 @"?seh_in_cxx@@YAHPEAH@Z"(ptr noundef %p)
// CHECK-SAME: #[[ATTR:[0-9]+]] personality ptr @__C_specific_handler
// CHECK: invoke void @llvm.seh.try.begin()
// CHECK: load volatile i32, ptr
// CHECK: invoke void @llvm.seh.try.end()
// CHECK: catchpad within %{{[^ ]+}} [ptr null]
// CHECK: store i32 -1, ptr %r

// CHECK-NOT: llvm.seh.scope
// CHECK: attributes #[[NOATTR]] = {
// CHECK-NOT: "seh-async-try"
// CHECK-SAME: }
// CHECK: attributes #[[ATTR]] = { {{.*}}"seh-async-try"{{.*}} }
// CHECK-NOT: "eh-asynch"
