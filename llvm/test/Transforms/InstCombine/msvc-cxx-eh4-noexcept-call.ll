; RUN: opt -passes=instcombine -S < %s | FileCheck %s

; A nounwind caller marks its calls nounwind, except in an FH4 noexcept
; function: there a call may unwind to the frame handler, which terminates,
; and the inliner keeps that as a terminate scope (openrt
; FH4-NOEXCEPT-TERMINATE).

declare void @may_throw()

; CHECK-LABEL: define void @plain_nounwind()
; CHECK: call void @may_throw() #[[NOUNWIND:[0-9]+]]
define void @plain_nounwind() nounwind {
  call void @may_throw()
  ret void
}

; CHECK-LABEL: define void @fh4_noexcept()
; CHECK: call void @may_throw(){{$}}
define void @fh4_noexcept() #0 {
  call void @may_throw()
  ret void
}

; CHECK: attributes #[[NOUNWIND]] = { nounwind }

attributes #0 = { nounwind "msvc-cxx-eh4-noexcept" }
