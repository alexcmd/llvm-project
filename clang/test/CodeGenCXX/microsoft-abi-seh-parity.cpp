// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -std=c++17 -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -std=c++17 -fasync-exceptions -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -std=c++17 -O2 -emit-llvm -o - %s | FileCheck %s --check-prefix=O2
// RUN: %clang_cc1 -triple aarch64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -std=c++17 -emit-llvm -o - %s | FileCheck %s --check-prefix=A64

// openrt SEH-PARITY (cl 14.51 x64, /EHsc and /EHa):
// - an abnormal exit of a __try (return/break/continue/goto, not __leave or
//   the fall-through) runs the __finally through _local_unwind; the call is
//   marked "msvc-seh-local-unwind" for the backend and the __finally stays
//   out of line;
// - a noexcept function with __try uses __C_specific_handler_noexcept and has
//   no terminate scope (structured exceptions pass through it); throw() and
//   __declspec(nothrow) keep __C_specific_handler, also without one.

extern "C" void work(int);
extern "C" void note(int);

int ret_in_try(int x) {
  __try {
    work(x);
    if (x)
      return 1;
  } __finally {
    note(x);
  }
  return 0;
}

int leave_only(int x) {
  __try {
    work(x);
    if (x)
      __leave;
  } __finally {
    note(x);
  }
  return 0;
}

void nx_fin(int x) noexcept {
  __try {
    work(x);
  } __finally {
    note(x);
  }
}

void throw_fin(int x) throw() {
  __try {
    work(x);
  } __finally {
    note(x);
  }
}

__declspec(nothrow) void nothrow_fin(int x) {
  __try {
    work(x);
  } __finally {
    note(x);
  }
}

// CHECK-LABEL: define dso_local noundef i32 @"?ret_in_try@@YAHH@Z"
// CHECK-SAME: personality ptr @__C_specific_handler {
// CHECK: br i1 %{{.*}}, label %seh.finally.local_unwind, label %seh.finally.direct
// CHECK: seh.finally.local_unwind:
// CHECK-NEXT: notail call void @"?fin$0@0@ret_in_try@@"(i8 noundef 1, ptr noundef %{{.*}}) #[[LU:[0-9]+]]
// CHECK: seh.finally.direct:
// CHECK-NEXT: call void @"?fin$0@0@ret_in_try@@"(i8 noundef 0, ptr noundef %{{.*}})
// CHECK-NOT: #[[LU]]
// CHECK: define internal void @"?fin$0@0@ret_in_try@@"(i8 noundef %abnormal_termination, ptr noundef %frame_pointer) #[[FIN:[0-9]+]]

// CHECK-LABEL: define dso_local noundef i32 @"?leave_only@@YAHH@Z"
// CHECK-NOT: msvc-seh-local-unwind
// CHECK-NOT: seh.finally.local_unwind
// CHECK: define internal void @"?fin$0@0@leave_only@@"

// CHECK-LABEL: define dso_local void @"?nx_fin@@YAXH@Z"
// CHECK-SAME: personality ptr @__C_specific_handler_noexcept {
// CHECK-NOT: terminate
// CHECK: ret void
// CHECK-LABEL: define dso_local void @"?throw_fin@@YAXH@Z"
// CHECK-SAME: personality ptr @__C_specific_handler {
// CHECK-NOT: terminate
// CHECK: ret void
// CHECK-LABEL: define dso_local void @"?nothrow_fin@@YAXH@Z"
// CHECK-SAME: personality ptr @__C_specific_handler {
// CHECK-NOT: terminate
// CHECK: ret void

// CHECK-DAG: attributes #[[FIN]] = { noinline
// CHECK-DAG: attributes #[[LU]] = { noinline "msvc-seh-local-unwind" }

// O2-LABEL: define dso_local {{.*}}i32 @"?ret_in_try@@YAHH@Z"
// O2: notail call {{.*}}void @"?fin$0@0@ret_in_try@@"({{.*}}) #[[LU2:[0-9]+]]
// O2: attributes #[[LU2]] = { noinline "msvc-seh-local-unwind" }

// Other targets keep the direct call.
// A64-NOT: msvc-seh-local-unwind
// A64-NOT: __C_specific_handler_noexcept
