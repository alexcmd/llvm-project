; RUN: llc < %s -mtriple=x86_64-pc-windows-msvc -O2 | FileCheck %s
; RUN: llc < %s -mtriple=x86_64-pc-windows-msvc -O0 | FileCheck %s
; RUN: llc < %s -mtriple=x86_64-pc-windows-msvc -O2 -filetype=obj -o %t.obj
; RUN: llvm-nm -u %t.obj | FileCheck %s --check-prefix=NM

; openrt SEH-PARITY (cl 14.51 x64): a direct call to an outlined __finally
; marked "msvc-seh-local-unwind" (abnormal exit of the __try) is emitted as
; cl does it, a call to _local_unwind(establisher frame, target); its scope
; entry [begin, target) comes first in the table and names the __finally's
; cleanup funclet. Without a cleanup funclet that calls the __finally the call
; stays direct. __C_specific_handler_noexcept is a table-SEH personality.

declare i32 @__C_specific_handler(...)
declare i32 @__C_specific_handler_noexcept(...)
declare void @work(i32)
declare void @note(i32)
declare ptr @llvm.localaddress()
declare ptr @llvm.localrecover(ptr, ptr, i32 immarg)
declare void @llvm.localescape(...)

define i32 @ret_in_try(i32 %x) personality ptr @__C_specific_handler {
entry:
  %x.addr = alloca i32, align 4
  call void (...) @llvm.localescape(ptr %x.addr)
  store i32 %x, ptr %x.addr, align 4
  invoke void @work(i32 %x)
          to label %cont unwind label %ehcleanup

cont:
  %abn = icmp ne i32 %x, 0
  %fp = call ptr @llvm.localaddress()
  br i1 %abn, label %lu, label %direct

lu:
  notail call void @fin(i8 1, ptr %fp) #0
  br label %done

direct:
  call void @fin(i8 0, ptr %fp)
  br label %done

done:
  %r = zext i1 %abn to i32
  ret i32 %r

ehcleanup:
  %pad = cleanuppad within none []
  %fp2 = call ptr @llvm.localaddress()
  call void @fin(i8 1, ptr %fp2) [ "funclet"(token %pad) ]
  cleanupret from %pad unwind to caller
}

define internal void @fin(i8 %abnormal, ptr %frame) noinline {
entry:
  %x.addr = call ptr @llvm.localrecover(ptr @ret_in_try, ptr %frame, i32 0)
  %v = load i32, ptr %x.addr, align 4
  call void @note(i32 %v)
  ret void
}

; CHECK-LABEL: ret_in_try:
; CHECK: .seh_handler __C_specific_handler, @unwind, @except
; CHECK: .seh_setframe %rbp, [[OFF:[0-9]+]]
; CHECK: [[BEGIN:\.Lseh_lu_begin[0-9]+]]:
; CHECK-NEXT: leaq -[[OFF]](%rbp), %rcx
; CHECK-NEXT: leaq [[TARGET:\.Lseh_lu_target[0-9]+]](%rip), %rdx
; CHECK-NEXT: callq _local_unwind
; CHECK-NEXT: nop
; CHECK-NEXT: [[TARGET]]:
; CHECK: .seh_handlerdata
; CHECK: .long (.Llsda_end{{[0-9]+}}-.Llsda_begin{{[0-9]+}})/16
; CHECK-NEXT: .Llsda_begin{{[0-9]+}}:
; CHECK-NEXT: .long [[BEGIN]]@IMGREL # LabelStart
; CHECK-NEXT: .long [[TARGET]]@IMGREL # LabelEnd
; CHECK-NEXT: .long "?dtor$[[DTOR:[0-9]+]]@?0?ret_in_try@4HA"@IMGREL # FinallyFunclet
; CHECK-NEXT: .long 0 # Null
; CHECK-NEXT: .long .Ltmp{{[0-9]+}}@IMGREL # LabelStart
; CHECK-NEXT: .long .Ltmp{{[0-9]+}}@IMGREL # LabelEnd
; CHECK-NEXT: .long "?dtor$[[DTOR]]@?0?ret_in_try@4HA"@IMGREL # FinallyFunclet

; No cleanup funclet: the call stays direct.
define i32 @no_funclet(i32 %x) {
entry:
  %x.addr = alloca i32, align 4
  call void (...) @llvm.localescape(ptr %x.addr)
  %fp = call ptr @llvm.localaddress()
  notail call void @fin2(i8 1, ptr %fp) #0
  ret i32 0
}

define internal void @fin2(i8 %abnormal, ptr %frame) noinline {
entry:
  %x.addr = call ptr @llvm.localrecover(ptr @no_funclet, ptr %frame, i32 0)
  %v = load i32, ptr %x.addr, align 4
  call void @note(i32 %v)
  ret void
}

; CHECK-LABEL: no_funclet:
; CHECK-NOT: _local_unwind
; CHECK: callq fin2
; CHECK-NOT: _local_unwind
; CHECK: .seh_endproc

define void @nx(i32 %x) nounwind personality ptr @__C_specific_handler_noexcept {
entry:
  invoke void @work(i32 %x)
          to label %cont unwind label %ehcleanup
cont:
  ret void
ehcleanup:
  %pad = cleanuppad within none []
  call void @note(i32 %x) [ "funclet"(token %pad) ]
  cleanupret from %pad unwind to caller
}

; CHECK-LABEL: nx:
; CHECK: .seh_handler __C_specific_handler_noexcept, @unwind, @except
; CHECK: .seh_handlerdata
; CHECK: .long (.Llsda_end{{[0-9]+}}-.Llsda_begin{{[0-9]+}})/16
; CHECK-NEXT: .Llsda_begin{{[0-9]+}}:
; CHECK-NEXT: .long .Ltmp{{[0-9]+}}@IMGREL # LabelStart

; NM: U __C_specific_handler
; NM: U __C_specific_handler_noexcept
; NM: U _local_unwind

attributes #0 = { noinline "msvc-seh-local-unwind" }
