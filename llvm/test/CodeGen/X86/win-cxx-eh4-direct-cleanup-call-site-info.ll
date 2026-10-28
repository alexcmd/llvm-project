; RUN: llc -mtriple=x86_64-pc-windows-msvc -O2 -emit-call-site-info < %s | FileCheck %s
;
; FH4 replaces a cleanup funclet that only destroys a frame object with a
; direct destructor action and deletes the funclet's machine blocks. With call
; site info (clang -O2 -g / clang-cl /O2 /Z7), the destructor call in that
; funclet has a call site info entry which must be erased with it, or deleting
; the instruction asserts ("Call site info was not updated!").

%struct.S = type { i32 }

declare void @g(i32)
declare void @"??1S@@QEAA@XZ"(ptr) nounwind
declare i32 @__CxxFrameHandler4(...)

define void @f(i32 %x) personality ptr @__CxxFrameHandler4 !dbg !5 {
entry:
  %s = alloca %struct.S, align 4
  store i32 %x, ptr %s, align 4
  invoke void @g(i32 %x)
          to label %cont unwind label %ehcleanup, !dbg !8
cont:
  call void @"??1S@@QEAA@XZ"(ptr %s), !dbg !8
  ret void
ehcleanup:
  %p = cleanuppad within none []
  call void @"??1S@@QEAA@XZ"(ptr %s) [ "funclet"(token %p) ], !dbg !8
  cleanupret from %p unwind to caller
}

; The cleanup funclet is gone; the unwind map destroys %s directly.
; CHECK-LABEL: f:
; CHECK:       .seh_handler __CxxFrameHandler4
; CHECK:       callq g
; CHECK-NOT:   "?dtor$
; CHECK:       $stateUnwindMap$f:
; CHECK:       .long "??1S@@QEAA@XZ"@IMGREL

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, isOptimized: true, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "t.cpp", directory: "")
!2 = !{}
!3 = !{i32 2, !"CodeView", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 3, type: !6, scopeLine: 3, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!6 = !DISubroutineType(types: !7)
!7 = !{null}
!8 = !DILocation(line: 3, scope: !5)
