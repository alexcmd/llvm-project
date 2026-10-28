; RUN: llc -mtriple=x86_64-pc-windows-msvc -O2 < %s | FileCheck %s
;
; FH4: a call that unwinds to the caller (no EH label) placed after the last
; invoke of the function needs its own ip2state reset to state -1. The shape is
; ix_throw_kind's switch: case 0 throws through a cleanup (state 0), case 1
; calls a noreturn thrower whose frame has nothing to destroy. Without the
; reset, ip2state leaves @raise in state 0 and unwinding through it runs the
; cleanup for an object that was never constructed.

declare void @may_throw()
declare void @raise() noreturn
declare void @dtor()
declare i32 @__CxxFrameHandler4(...)

define void @f(i32 %k) personality ptr @__CxxFrameHandler4 {
entry:
  %c = icmp eq i32 %k, 0
  br i1 %c, label %a, label %b, !prof !0
a:
  invoke void @may_throw()
          to label %unr unwind label %cl
unr:
  unreachable
cl:
  %p = cleanuppad within none []
  call void @dtor() [ "funclet"(token %p) ]
  cleanupret from %p unwind to caller
b:
  call void @raise()
  unreachable
}

!0 = !{!"branch_weights", i32 1000, i32 1}

; CHECK-LABEL: f:
; CHECK:       .seh_handler __CxxFrameHandler4, @unwind
; CHECK:       [[BEGIN:\.Ltmp[0-9]+]]: # EH_LABEL
; CHECK-NEXT:  callq may_throw
; CHECK-NEXT:  nop
; CHECK-NEXT:  [[END:\.Ltmp[0-9]+]]: # EH_LABEL
; CHECK:       callq raise
; CHECK:       $ip2state$f:
; CHECK-NEXT:  .byte 4 # NumIPMapEntries
; CHECK-NEXT:  .byte 15 # IPDelta
; CHECK-NEXT:  .long [[BEGIN]]-.Lfunc_begin0
; CHECK-NEXT:  .byte 2 # StatePlusOne
; CHECK-NEXT:  .byte 15 # IPDelta
; CHECK-NEXT:  .long [[END]]-[[BEGIN]]
; CHECK-NEXT:  .byte 0 # StatePlusOne
