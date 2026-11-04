; cl gives one flag per file in clang's list (llvm.msvc.jmc.files, cl order):
; the header flag is created and kept (llvm.used) even with no function in it,
; and the main file is last (its flag is kept too). The default check function is not pinned.
; RUN: opt -passes=jmc-instrumenter -S < %s | FileCheck %s

; CHECK: @"_{{[0-9A-F]+}}_hdr@h" = internal unnamed_addr global i8 1, section ".msvcjmc", align 1, !dbg !{{[0-9]+}}
; CHECK: @"_7DE10BAA_x@c" = internal unnamed_addr global i8 1, section ".msvcjmc", align 1, !dbg !{{[0-9]+}}
; CHECK: @llvm.used = appending global [2 x ptr] [ptr @"_{{[0-9A-F]+}}_hdr@h", ptr @"_7DE10BAA_x@c"], section "llvm.metadata"
; CHECK-NOT: @__JustMyCode_Default
; CHECK-NOT: /INCLUDE
; CHECK: define void @w1() #0 !dbg !{{[0-9]+}} {
; CHECK:   call x86_fastcallcc void @__CheckForDebuggerJustMyCode(ptr inreg noundef @"_7DE10BAA_x@c")
; CHECK: !llvm.msvc.jmc.files = !{!{{[0-9]+}}, !{{[0-9]+}}}

target datalayout = "e-m:x-p:32:32-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32-a:0:32-S32"
target triple = "i386-pc-windows-msvc"

define void @w1() #0 !dbg !10 {
  ret void
}

attributes #0 = { "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8}
!llvm.msvc.jmc.files = !{!20, !1}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "./b/./../b/x.c", directory: "C:\\\\a\\\\")
!20 = !DIFile(filename: "hdr.h", directory: "C:\\\\a\\\\")
!7 = !{i32 2, !"CodeView", i32 1}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!10 = distinct !DISubprogram(name: "w1", scope: !1, file: !1, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!31 = !DISubroutineType(types: !32)
!32 = !{null}
!33 = !{}
