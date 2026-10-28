; RUN: llc -mtriple=x86_64-pc-windows-msvc -O2 < %s | FileCheck %s
;
; FH4: two catch handlers returning to one block through a phi (the shape of
; try { ... } catch (A &) { x = 1; } catch (...) { x = 2; } use(x);). The catch
; funclet must return its continuation (not be emitted empty), the handler map
; must reference the funclet and a function-relative continuation, and the
; parent must use __CxxFrameHandler4 with the compact FuncInfo4 tables.

declare void @thrower()
declare i32 @__CxxFrameHandler4(...)

define void @use(i32 %x) {
  call void @thrower()
  ret void
}

; CHECK-LABEL: f:
; CHECK:       .seh_handler __CxxFrameHandler4, @unwind, @except
; CHECK:       callq thrower
; CHECK:       [[CONT:\.LBB[0-9_]+]]: # Block address taken
; CHECK-NEXT:  # %tail
; CHECK:       callq use
; CHECK:       "?catch$[[C:[0-9]+]]@?0?f@4HA":
; CHECK:       .seh_endprologue
; CHECK-NEXT:  xorl %eax, %eax
; CHECK:       retq # CATCHRET
; CHECK:       $ip2state$f:
; CHECK:       $handlerMap$0$f:
; CHECK:       .long "?catch$[[C]]@?0?f@4HA"@IMGREL # Handler
; CHECK-NEXT:  .byte 15 # Continuation
; CHECK-NEXT:  .long [[CONT]]-.Lfunc_begin0
; CHECK:       $tryMap$f:
; CHECK:       $cppxdata$f:
; CHECK-NEXT:  .byte 56 # FuncInfo4Header
define void @f() personality ptr @__CxxFrameHandler4 {
entry:
  invoke void @thrower() to label %unr unwind label %cs
cs:
  %sw = catchswitch within none [label %c1, label %c2] unwind to caller
c1:
  %p1 = catchpad within %sw [ptr null, i32 64, ptr null]
  catchret from %p1 to label %tail
c2:
  %p2 = catchpad within %sw [ptr null, i32 64, ptr null]
  catchret from %p2 to label %tail
tail:
  %v = phi i32 [ 1, %c1 ], [ 2, %c2 ]
  call void @use(i32 %v)
  ret void
unr:
  unreachable
}
