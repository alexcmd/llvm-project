; RUN: opt -passes=inline -S < %s | FileCheck %s

; A caller that inlines a function with the "seh-async-try" attribute gets it
; too, so the backend still tracks the EH state of the inlined __try body.

; CHECK-LABEL: define i32 @caller(ptr %p)
; CHECK-SAME: #[[ATTR:[0-9]+]] personality ptr @__C_specific_handler
; CHECK: invoke void @llvm.seh.try.begin()
; CHECK: load volatile i32, ptr %p
; CHECK: invoke void @llvm.seh.try.end()
; CHECK-LABEL: define i32 @plain_caller(ptr %p) {
; CHECK: attributes #[[ATTR]] = { "seh-async-try" }

define internal i32 @callee(ptr %p) #0 personality ptr @__C_specific_handler {
entry:
  invoke void @llvm.seh.try.begin()
          to label %invoke.cont unwind label %catch.dispatch

invoke.cont:
  %v = load volatile i32, ptr %p, align 4
  invoke void @llvm.seh.try.end()
          to label %try.cont unwind label %catch.dispatch

catch.dispatch:
  %cs = catchswitch within none [label %__except] unwind to caller

__except:
  %cp = catchpad within %cs [ptr null]
  catchret from %cp to label %except.body

except.body:
  br label %try.cont

try.cont:
  %r = phi i32 [ %v, %invoke.cont ], [ -1, %except.body ]
  ret i32 %r
}

define i32 @caller(ptr %p) {
  %r = call i32 @callee(ptr %p)
  ret i32 %r
}

define internal i32 @plain_callee(ptr %p) {
  %v = load i32, ptr %p, align 4
  ret i32 %v
}

define i32 @plain_caller(ptr %p) {
  %r = call i32 @plain_callee(ptr %p)
  ret i32 %r
}

declare void @llvm.seh.try.begin()
declare void @llvm.seh.try.end()
declare i32 @__C_specific_handler(...)

attributes #0 = { "seh-async-try" }
