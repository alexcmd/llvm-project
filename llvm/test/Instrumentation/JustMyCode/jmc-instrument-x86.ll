; RUN: opt -jmc-instrumenter -S < %s | FileCheck %s
; RUN: opt -passes=jmc-instrumenter -S < %s | FileCheck %s

; CHECK: $_JustMyCode_Default = comdat any

; CHECK: @"_7DE10BAA_x@c" = internal unnamed_addr global i8 1, section ".msvcjmc", align 1, !dbg !{{[0-9]+}}

; CHECK: define void @w1() #0 !dbg !{{[0-9]+}} {
; CHECK:   call x86_fastcallcc void @__CheckForDebuggerJustMyCode(ptr inreg noundef @"_7DE10BAA_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @_JustMyCode_Default(ptr inreg noundef %0) unnamed_addr comdat {
; CHECK:   ret void
; CHECK: }

; CHECK: declare x86_fastcallcc void @__CheckForDebuggerJustMyCode(ptr inreg noundef) unnamed_addr

; CHECK: !{{[0-9]+}} = !DIGlobalVariableExpression(var: !{{[0-9]+}}, expr: !DIExpression())
; CHECK: !{{[0-9]+}} = distinct !DIGlobalVariable(name: "_7DE10BAA_x@c", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, type: !{{[0-9]+}}, isLocal: true, isDefinition: true)
; CHECK: !{{[0-9]+}} = distinct !DICompileUnit(language: DW_LANG_C99, file: !{{[0-9]+}}, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !{{[0-9]+}}, splitDebugInlining: false, nameTableKind: None)
; CHECK: !{{[0-9]+}} = !DIFile(filename: "./b/./../b/x.c", directory: "C:\\\\a\\\\")
; CHECK: !{{[0-9]+}} = !{!{{[0-9]+}}}
; CHECK: !{{[0-9]+}} = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char, flags: DIFlagArtificial)
; CHECK: !{{[0-9]+}} = !{i32 2, !"CodeView", i32 1}
; CHECK: !{{[0-9]+}} = !{i32 2, !"Debug Info Version", i32 3}
; CHECK: !{{[0-9]+}} = !{!"clang"}
; CHECK: !{{[0-9]+}} = !{!"/alternatename:__CheckForDebuggerJustMyCode=_JustMyCode_Default"}
; CHECK: !{{[0-9]+}} = distinct !DISubprogram(name: "w1", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK: !{{[0-9]+}} = !DISubroutineType(types: !{{[0-9]+}})

target datalayout = "e-m:x-p:32:32-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32-a:0:32-S32"
target triple = "i386-pc-windows-msvc"

define void @w1() #0 !dbg !10 {
  ret void
}

attributes #0 = { "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "./b/./../b/x.c", directory: "C:\\\\a\\\\")
!7 = !{i32 2, !"CodeView", i32 1}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{!"clang"}
!10 = distinct !DISubprogram(name: "w1", scope: !1, file: !1, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!31 = !DISubroutineType(types: !32)
!32 = !{null}
!33 = !{}
