; RUN: opt -S -passes=callsite-splitting < %s | FileCheck %s
; RUN: opt -S -passes=callsite-splitting,win-eh-prepare < %s | FileCheck %s --check-prefix=PREP
;
; A call site whose predecessors end in catchret must not be split: the cloned
; calls would be placed inside the catch funclets without a "funclet" operand
; bundle, and WinEHPrepare would then replace them with unreachable.

target triple = "x86_64-pc-windows-msvc"

declare void @thrower()
declare i32 @__CxxFrameHandler3(...)

define void @use(i32 %x) {
  call void @thrower()
  ret void
}

; CHECK-LABEL: define void @f(
; CHECK:       c1:
; CHECK-NEXT:    catchpad
; CHECK-NEXT:    catchret from %p1 to label %tail
; CHECK:       c2:
; CHECK-NEXT:    catchpad
; CHECK-NEXT:    catchret from %p2 to label %tail
; CHECK:       tail:
; CHECK-NEXT:    %v = phi i32 [ 1, %c1 ], [ 2, %c2 ]
; CHECK-NEXT:    call void @use(i32 %v)
; CHECK-NOT:   call void @use

; PREP-LABEL: define void @f(
; PREP-NOT:   unreachable
; PREP:       catchret from %p1 to label %tail
; PREP-NOT:   unreachable
; PREP:       call void @use(i32
; PREP:       unr:
; PREP-NEXT:    unreachable
define void @f() personality ptr @__CxxFrameHandler3 {
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
