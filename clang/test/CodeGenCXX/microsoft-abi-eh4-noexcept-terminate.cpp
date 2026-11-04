// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -emit-llvm -o - %s | FileCheck %s --check-prefix=IR
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -O2 -emit-llvm -o - %s | FileCheck %s --check-prefix=O2
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -O2 -S -o - %s | FileCheck %s --check-prefix=ASM
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -fno-ms-cxx-eh4 -O2 -emit-llvm -o - %s | FileCheck %s --check-prefix=FH3

// openrt FH4-NOEXCEPT-TERMINATE (cl 14.51 /O2 /d2FH4 /EHsc): an inlined
// noexcept callee leaves one $stateUnwindMap terminate entry (RVA
// __std_terminate) in its caller, also when the caller is a leaf noexcept
// function (vcstartup tlsdyn.obj) or a deleting destructor (comsupp
// comraise.obj ??_G).

typedef void (*PVFV)();
extern "C" PVFV *xd_a, *xd_z;
extern "C" bool guard;

// Every source noexcept function carries the attribute, the leaf one too.
// IR: define {{.*}}void @walk() [[NOEXCEPT:#[0-9]+]]{{.*}} personality
extern "C" void walk() noexcept {
  if (guard)
    return;
  guard = true;
  for (PVFV *p = xd_a; p != xd_z; ++p)
    if (*p)
      (*p)();
}
// IR: define {{.*}}void @on_demand() [[NOEXCEPT]] {
extern "C" void on_demand() noexcept { walk(); }

// O2-LABEL: define {{.*}}void @on_demand()
// O2-SAME: personality ptr @__CxxFrameHandler4
// O2: invoke void %{{.*}}()
// O2-NEXT: to label %{{.*}} unwind label %[[TERM:[^ ,]+]]
// O2: [[TERM]]:
// O2-NEXT: %[[P:.*]] = cleanuppad within none []
// O2-NEXT: call void @__std_terminate() {{.*}}[ "funclet"(token %[[P]]) ]
// O2-NEXT: unreachable

// ASM-LABEL: on_demand:
// ASM: .seh_handler __CxxFrameHandler4, @unwind, @except
// ASM-NOT: callq __std_terminate
// ASM: $stateUnwindMap$on_demand:
// ASM-NEXT: .byte 2{{.*}}# NumUnwindEntries
// ASM: .long __std_terminate@IMGREL{{.*}}# UnwindAction
// ASM-NEXT: $cppxdata$on_demand:
// ASM-NEXT: .byte 104{{.*}}# FuncInfo4Header

struct I { virtual void Release(); };
struct E {
  I *q;
  virtual ~E() { if (q) q->Release(); }
  E();
};
E::E() {}

// The scalar deleting destructor inlines ~E() under a terminate state.
// ASM-LABEL: "??_GE@@UEAAPEAXI@Z":
// ASM: .seh_handler __CxxFrameHandler4, @unwind, @except
// ASM: "$stateUnwindMap$??_GE@@UEAAPEAXI@Z":
// ASM-NEXT: .byte 2{{.*}}# NumUnwindEntries
// ASM: .long __std_terminate@IMGREL{{.*}}# UnwindAction
// ASM-NEXT: "$cppxdata$??_GE@@UEAAPEAXI@Z":
// ASM-NEXT: .byte 104{{.*}}# FuncInfo4Header

// IR: attributes [[NOEXCEPT]] = { {{.*}}"msvc-cxx-eh4-noexcept"

// FH3 keeps clang's terminate funclet and no attribute.
// FH3-NOT: "msvc-cxx-eh4-noexcept"
