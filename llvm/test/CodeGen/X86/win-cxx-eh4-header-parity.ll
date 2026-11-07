; RUN: split-file %s %t
; RUN: llc < %t/ehs.ll -mtriple=x86_64-pc-windows-msvc -O2 | FileCheck %s --check-prefix=EHS
; RUN: llc < %t/eha.ll -mtriple=x86_64-pc-windows-msvc -O2 | FileCheck %s --check-prefix=EHA

; openrt FH4-HEADER-PARITY (cl 14.51 /d2FH4): a FuncInfo4 record without
; states has a one-entry IPtoStateMap (null state from the function start),
; never an empty one; a noexcept function whose remaining calls all go to
; nounwind functions has no record (cl /O2 /EHsc); /EHa keeps the NoExcept bit.

;--- ehs.ll
declare i32 @__CxxFrameHandler4(...)
declare void @may_throw()
declare void @cannot_throw() nounwind

define void @noexcept_call() #0 personality ptr @__CxxFrameHandler4 {
entry:
  call void @may_throw()
  call void @cannot_throw()
  ret void
}
; EHS-LABEL: noexcept_call:
; EHS: .seh_handler __CxxFrameHandler4, @unwind, @except
; EHS-LABEL: $ip2state$noexcept_call:
; EHS-NEXT: .byte 2{{ +}}# NumIPMapEntries
; EHS-NEXT: .byte 0{{ +}}# IPDelta
; EHS-NEXT: .byte 0{{ +}}# StatePlusOne
; EHS-NEXT: $cppxdata$noexcept_call:
; EHS-NEXT: .byte 96{{ +}}# FuncInfo4Header
; EHS-NEXT: .long $ip2state$noexcept_call@IMGREL # IPToStateMap

; Only a nounwind callee is left: no handler, no record.
define void @noexcept_nounwind_callee() #0 personality ptr @__CxxFrameHandler4 {
entry:
  call void @cannot_throw()
  call void @cannot_throw()
  ret void
}
; EHS-LABEL: noexcept_nounwind_callee:
; EHS-NOT: .seh_handler
; EHS-NOT: $cppxdata$noexcept_nounwind_callee
; EHS: .seh_endproc

; optnone (cl /Od): any call keeps the record.
define void @noexcept_nounwind_callee_optnone() #1 personality ptr @__CxxFrameHandler4 {
entry:
  call void @cannot_throw()
  ret void
}
; EHS-LABEL: noexcept_nounwind_callee_optnone:
; EHS: .seh_handler __CxxFrameHandler4, @unwind, @except
; EHS-LABEL: $cppxdata$noexcept_nounwind_callee_optnone:
; EHS-NEXT: .byte 96{{ +}}# FuncInfo4Header

; A call site made nounwind (inlining into a nounwind caller) to a function
; that can throw still needs the FH4 frame.
define void @noexcept_nounwind_site() #0 personality ptr @__CxxFrameHandler4 {
entry:
  call void @may_throw() nounwind
  call void @cannot_throw()
  ret void
}
; EHS-LABEL: noexcept_nounwind_site:
; EHS: .seh_handler __CxxFrameHandler4, @unwind, @except
; EHS-LABEL: $cppxdata$noexcept_nounwind_site:
; EHS-NEXT: .byte 96{{ +}}# FuncInfo4Header

attributes #0 = { nounwind uwtable "msvc-cxx-eh4-noexcept" }
attributes #1 = { noinline optnone nounwind uwtable "msvc-cxx-eh4-noexcept" }

;--- eha.ll
declare i32 @__CxxFrameHandler4(...)
declare void @may_throw()

define void @noexcept_eha() #0 personality ptr @__CxxFrameHandler4 {
entry:
  call void @may_throw()
  ret void
}
; EHA-LABEL: noexcept_eha:
; EHA: .seh_handler __CxxFrameHandler4, @unwind, @except
; EHA-LABEL: $cppxdata$noexcept_eha:
; EHA-NEXT: .byte 64{{ +}}# FuncInfo4Header
; EHA-NEXT: .long $ip2state$noexcept_eha@IMGREL # IPToStateMap

attributes #0 = { nounwind uwtable "msvc-cxx-eh4-noexcept" }

!llvm.module.flags = !{!0}
!0 = !{i32 2, !"eh-asynch", i32 1}
