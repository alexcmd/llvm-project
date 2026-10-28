; RUN: llc -mtriple=x86_64-pc-windows-msvc -O0 < %s | FileCheck %s
; RUN: llc -mtriple=x86_64-pc-windows-msvc -O2 < %s | FileCheck %s

; The "seh-async-try" function attribute (clang -fms-seh-async-try) turns on
; the -EHa IP-to-state tracking for one function without the "eh-asynch"
; module flag, so the __C_specific_handler scope table covers the faulting
; load inside llvm.seh.try.begin/end.

; CHECK-LABEL: fault:
; CHECK: .seh_handler __C_specific_handler, @unwind, @except
; CHECK: [[BEGIN:\.Ltmp[0-9]+]]:{{.*}}EH_LABEL
; CHECK-NEXT: nop
; CHECK: movl (%r{{[a-z]+}}), %eax
; CHECK-NEXT: [[END:\.Ltmp[0-9]+]]:{{.*}}EH_LABEL
; CHECK: [[EXCEPT:\.LBB0_[0-9]+]]:{{.*}}%__except
; CHECK: .seh_handlerdata
; CHECK-NEXT: .Lfault$parent_frame_offset =
; CHECK-NEXT: .long (.Llsda_end0-.Llsda_begin0)/16
; CHECK-NEXT: .Llsda_begin0:
; CHECK-NEXT: .long [[BEGIN]]@IMGREL
; CHECK-NEXT: .long [[END]]@IMGREL
; CHECK-NEXT: .long 1
; CHECK-NEXT: .long [[EXCEPT]]@IMGREL
; CHECK-NEXT: .Llsda_end0:
; CHECK: .seh_endproc

define dso_local i32 @fault(ptr %p) #0 personality ptr @__C_specific_handler {
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

; Without the attribute (and without "eh-asynch") only invokes of real calls
; get scope entries: the load is not covered and the table is empty.

; CHECK-LABEL: no_attr:
; CHECK: .seh_handler __C_specific_handler, @unwind, @except
; CHECK-NOT: EH_LABEL
; CHECK: .seh_handlerdata
; CHECK-NEXT: .Lno_attr$parent_frame_offset =
; CHECK-NEXT: .long (.Llsda_end1-.Llsda_begin1)/16
; CHECK-NEXT: .Llsda_begin1:
; CHECK-NEXT: .Llsda_end1:
; CHECK: .seh_endproc

define dso_local i32 @no_attr(ptr %p) personality ptr @__C_specific_handler {
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

declare dso_local void @llvm.seh.try.begin()
declare dso_local void @llvm.seh.try.end()
declare dso_local i32 @__C_specific_handler(...)

attributes #0 = { "seh-async-try" }
