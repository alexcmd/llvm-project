; RUN: llc < %s -mtriple=x86_64-pc-windows-msvc -O2 | FileCheck %s

; openrt FH4-NOEXCEPT-TERMINATE: a terminate pad (cleanuppad; call
; __std_terminate; unreachable or cleanupret) is cl /d2FH4's terminate state:
; $stateUnwindMap RVA action __std_terminate, no object word, no funclet code.
; Without any funclet left the frame is plain (no frame pointer, no UnwindHelp),
; as cl: "sub rsp, 40 / call / npad 1 / add rsp, 40 / jmp".

; CHECK-LABEL: region:
; CHECK: .seh_handler __CxxFrameHandler4, @unwind{{[[:space:]]*$}}
; CHECK-NEXT: # %bb.0:
; CHECK-NEXT: subq $40, %rsp
; CHECK-NOT: %rbp
; CHECK: callq may_throw
; CHECK-NEXT: nop
; CHECK: addq $40, %rsp
; CHECK: jmp may_throw
; CHECK-NOT: __std_terminate
; CHECK-LABEL: $ip2state$region:
; One entry: the tail call leaves the frame, no final reset.
; CHECK-NEXT: .byte 2{{.*}}# NumIPMapEntries
; CHECK-NEXT: .byte 15{{.*}}# IPDelta
; CHECK-NEXT: .long .Ltmp0-.Lfunc_begin0
; CHECK-NEXT: .byte 2{{.*}}# StatePlusOne
; CHECK-NEXT: $stateUnwindMap$region:
; CHECK-NEXT: .byte 2{{.*}}# NumUnwindEntries
; CHECK-NEXT: .Lfh4_uw_begin0:
; CHECK-NEXT: .byte 15{{.*}}# UnwindEntry
; CHECK-NEXT: .long ((.Lfh4_uw_begin0-.Lfh4_uw_begin0+1)*4)+3
; CHECK-NEXT: .long __std_terminate@IMGREL{{.*}}# UnwindAction
; CHECK-NEXT: $cppxdata$region:
; CHECK-NEXT: .byte 40{{.*}}# FuncInfo4Header

; CHECK-LABEL: noexcept_region:
; CHECK: .seh_handler __CxxFrameHandler4, @unwind, @except
; CHECK-NOT: __std_terminate
; CHECK: $stateUnwindMap$noexcept_region:
; CHECK: .long __std_terminate@IMGREL{{.*}}# UnwindAction
; CHECK-NEXT: $cppxdata$noexcept_region:
; CHECK-NEXT: .byte 104{{.*}}# FuncInfo4Header

; CHECK-LABEL: nested:
; CHECK-NEXT: .Lfunc_begin2:
; CHECK-NOT: "?dtor$
; CHECK: $stateUnwindMap$nested:
; CHECK-NEXT: .byte 4{{.*}}# NumUnwindEntries
; CHECK-NEXT: .Lfh4_uw_begin2:
; CHECK-NEXT: .byte 15{{.*}}# UnwindEntry
; CHECK-NEXT: .long ((.Lfh4_uw_begin2-.Lfh4_uw_begin2+1)*4)+1
; CHECK-NEXT: .long destroy@IMGREL{{.*}}# UnwindAction
; CHECK-NEXT: .byte {{[0-9]+}}{{.*}}# UnwindObject
; CHECK-NEXT: .Lfh4_uw_entry0:
; CHECK-NEXT: .byte 15{{.*}}# UnwindEntry
; ToState = the destructor state (entry 0).
; CHECK-NEXT: .long ((.Lfh4_uw_entry0-.Lfh4_uw_begin2)*4)+3
; CHECK-NEXT: .long __std_terminate@IMGREL{{.*}}# UnwindAction
; CHECK-NEXT: $cppxdata$nested:
; CHECK-NEXT: .byte 40{{.*}}# FuncInfo4Header

declare i32 @__CxxFrameHandler4(...)
declare void @may_throw()
declare void @__std_terminate()
declare void @destroy(ptr)

; cl /O2 /d2FH4: an inlined noexcept region in a caller that is not noexcept.
define void @region() personality ptr @__CxxFrameHandler4 {
entry:
  invoke void @may_throw()
          to label %cont unwind label %terminate
cont:
  tail call void @may_throw()
  ret void
terminate:
  %p = cleanuppad within none []
  call void @__std_terminate() #1 [ "funclet"(token %p) ]
  unreachable
}

; The same region in a noexcept caller: FuncInfo4 NoExcept bit too.
define void @noexcept_region() #0 personality ptr @__CxxFrameHandler4 {
entry:
  invoke void @may_throw()
          to label %cont unwind label %terminate
cont:
  call void @may_throw()
  ret void
terminate:
  %p = cleanuppad within none []
  call void @__std_terminate() #1 [ "funclet"(token %p) ]
  unreachable
}

; Terminate state nested in a destructor state (ToState = that state).
define void @nested() personality ptr @__CxxFrameHandler4 {
entry:
  %d = alloca i32, align 4
  invoke void @may_throw()
          to label %a unwind label %dtor
a:
  invoke void @may_throw()
          to label %b unwind label %term
b:
  call void @destroy(ptr %d)
  ret void
term:
  %p = cleanuppad within none []
  call void @__std_terminate() #2 [ "funclet"(token %p) ]
  cleanupret from %p unwind label %dtor
dtor:
  %q = cleanuppad within none []
  call void @destroy(ptr %d) #2 [ "funclet"(token %q) ]
  cleanupret from %q unwind to caller
}

attributes #0 = { nounwind "msvc-cxx-eh4-noexcept" }
attributes #1 = { noreturn nounwind }
attributes #2 = { nounwind }
