; RUN: opt -S -passes=argpromotion < %s | FileCheck %s

; Argument promotion moves the callee's loads into the caller in front of the
; call. If the call site is an invoke under an asynchronous (SEH) personality,
; or the caller tracks the EH state of every instruction ("seh-async-try"
; attribute, "eh-asynch" module flag), a fault in such a load must be caught
; by the handler of the invoke; hoisted out of the callee it would land
; outside the invoke's EH range. Such callees are not promoted.

target triple = "x86_64-pc-windows-msvc"

; SEH __try/__except around a call: __C_specific_handler.

define internal i32 @seh_callee(ptr %p) {
; CHECK-LABEL: define internal i32 @seh_callee(ptr %p)
; CHECK-NEXT: %v = load i32, ptr %p
  %v = load i32, ptr %p
  ret i32 %v
}

define i32 @seh_caller(ptr %p) personality ptr @__C_specific_handler {
; CHECK-LABEL: define i32 @seh_caller(ptr %p)
; CHECK-NEXT: entry:
; CHECK-NEXT: %r = invoke i32 @seh_callee(ptr %p)
entry:
  %r = invoke i32 @seh_callee(ptr %p)
          to label %cont unwind label %dispatch

cont:
  ret i32 %r

dispatch:
  %cs = catchswitch within none [label %handler] unwind to caller

handler:
  %cp = catchpad within %cs [ptr null]
  catchret from %cp to label %except

except:
  ret i32 -1
}

; C++ EH (synchronous personality): a fault is not an exception, promotion is
; still done.

define internal i32 @cxx_callee(ptr %p) {
; CHECK-LABEL: define internal i32 @cxx_callee(i32 %p.0.val)
  %v = load i32, ptr %p
  ret i32 %v
}

define i32 @cxx_caller(ptr %p) personality ptr @__CxxFrameHandler3 {
; CHECK-LABEL: define i32 @cxx_caller(ptr %p)
; CHECK: %p.val = load i32, ptr %p
; CHECK-NEXT: %r = invoke i32 @cxx_callee(i32 %p.val)
entry:
  %r = invoke i32 @cxx_callee(ptr %p)
          to label %cont unwind label %dispatch

cont:
  ret i32 %r

dispatch:
  %cs = catchswitch within none [label %handler] unwind to caller

handler:
  %cp = catchpad within %cs [ptr null, i32 64, ptr null]
  catchret from %cp to label %except

except:
  ret i32 -1
}

; A plain call (not in a __try) in a function with an SEH personality is
; still promoted.

define internal i32 @plain_callee(ptr %p) {
; CHECK-LABEL: define internal i32 @plain_callee(i32 %p.0.val)
  %v = load i32, ptr %p
  ret i32 %v
}

define i32 @plain_caller(ptr %p) personality ptr @__C_specific_handler {
; CHECK-LABEL: define i32 @plain_caller(ptr %p)
; CHECK: %p.val = load i32, ptr %p
; CHECK-NEXT: %r = call i32 @plain_callee(i32 %p.val)
  %r = call i32 @plain_callee(ptr %p)
  ret i32 %r
}

; clang -fms-seh-async-try: the caller's __try tracks every instruction.

define internal i32 @async_callee(ptr %p) {
; CHECK-LABEL: define internal i32 @async_callee(ptr %p)
  %v = load i32, ptr %p
  ret i32 %v
}

define i32 @async_caller(ptr %p) #0 personality ptr @__CxxFrameHandler3 {
; CHECK-LABEL: define i32 @async_caller(ptr %p)
; CHECK-NEXT: entry:
; CHECK-NEXT: %r = invoke i32 @async_callee(ptr %p)
entry:
  %r = invoke i32 @async_callee(ptr %p)
          to label %cont unwind label %dispatch

cont:
  ret i32 %r

dispatch:
  %cs = catchswitch within none [label %handler] unwind to caller

handler:
  %cp = catchpad within %cs [ptr null, i32 64, ptr null]
  catchret from %cp to label %except

except:
  ret i32 -1
}

declare i32 @__C_specific_handler(...)
declare i32 @__CxxFrameHandler3(...)

attributes #0 = { "seh-async-try" }
