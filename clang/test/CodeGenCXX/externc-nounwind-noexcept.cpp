// RUN: %clang_cc1 -triple x86_64-windows-msvc -fexceptions -fcxx-exceptions -fexternc-nounwind -std=c++14 -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,CXX14
// RUN: %clang_cc1 -triple x86_64-windows-msvc -fexceptions -fcxx-exceptions -fexternc-nounwind -std=c++17 -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,CXX17

// -fexternc-nounwind (clang-cl /EHsc) only assumes that extern "C" functions
// *without* an exception specification do not throw. An extern "C" definition
// that has an explicit noexcept keeps its terminate scope, exactly like a C++
// noexcept function; this matches cl /EHsc, which emits an FH4 noexcept state
// for it. throw() follows the language mode (no-op before C++17 in the MS ABI,
// noexcept from C++17 on), also matching cl.

void may_throw();
extern "C" void ext_decl();
struct Cleanup { ~Cleanup(); };

// CHECK-LABEL: define dso_local void @def_noexcept()
// CHECK-SAME: personality ptr @__CxxFrameHandler3
// CHECK: invoke void @"?may_throw@@YAXXZ"()
// CHECK-NEXT: to label %{{.*}} unwind label %[[TERM:.*]]
// CHECK: [[TERM]]:
// CHECK-NEXT: %[[PAD:.*]] = cleanuppad within none []
// CHECK-NEXT: call void @"?terminate@@YAXXZ"() #{{[0-9]+}} [ "funclet"(token %[[PAD]]) ]
extern "C" void def_noexcept() noexcept { may_throw(); }

// CXX14-LABEL: define dso_local void @def_throw()
// CXX14-NOT: personality
// CXX14: call void @"?may_throw@@YAXXZ"()
// CXX17-LABEL: define dso_local void @def_throw()
// CXX17-SAME: personality ptr @__CxxFrameHandler3
// CXX17: invoke void @"?may_throw@@YAXXZ"()
// CXX17: call void @"?terminate@@YAXXZ"()
extern "C" void def_throw() throw() { may_throw(); }

// No exception specification: implicitly nounwind, no EH.
// CHECK-LABEL: define dso_local void @def_plain()
// CHECK-NOT: personality
// CHECK: call void @"?may_throw@@YAXXZ"()
// CHECK-NEXT: ret void
extern "C" void def_plain() { may_throw(); }

// Same terminate scope as the extern "C" noexcept definition.
// CHECK-LABEL: define dso_local void @"?cxx_noexcept@@YAXXZ"()
// CHECK-SAME: personality ptr @__CxxFrameHandler3
// CHECK: invoke void @"?may_throw@@YAXXZ"()
// CHECK: call void @"?terminate@@YAXXZ"()
void cxx_noexcept() noexcept { may_throw(); }

// Calls to extern "C" functions stay nounwind: no cleanup landing pad.
// CHECK-LABEL: define dso_local void @"?call_externc_decl@@YAXXZ"()
// CHECK-NOT: personality
// CHECK: call void @ext_decl() #[[NUW:[0-9]+]]
// CHECK-NEXT: call void @"??1Cleanup@@QEAA@XZ"
void call_externc_decl() { Cleanup c; ext_decl(); }

// CHECK-LABEL: define dso_local void @"?call_externc_def@@YAXXZ"()
// CHECK-NOT: personality
// CHECK: call void @def_plain() #[[NUW]]
void call_externc_def() { Cleanup c; def_plain(); }

// CHECK-LABEL: define dso_local void @"?call_externc_noexcept@@YAXXZ"()
// CHECK-NOT: personality
// CHECK: call void @def_noexcept() #[[NUW]]
void call_externc_noexcept() { Cleanup c; def_noexcept(); }

// CHECK: attributes #[[NUW]] = { nounwind }
