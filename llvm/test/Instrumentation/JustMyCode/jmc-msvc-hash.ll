; RUN: opt -passes=jmc-instrumenter -mtriple=x86_64-pc-windows-msvc -S < %s | FileCheck %s
; cl /JMC flag names (tasks/compat/data/jmc_hash.md, probes c01..c25): CRC-32 of the full path,
; fname+ext with '.' -> '@'. Each expected name below is a probe result from real cl 14.51.

; CHECK-DAG: @"__233DDC46_jmc@cpp" = internal unnamed_addr global i8 1, section ".msvcjmc"
; CHECK-DAG: @"__1AB0E083_jmc@cpp" = internal unnamed_addr global i8 1, section ".msvcjmc"
; CHECK-DAG: @"__BC4A8D62_JMC@CPP" = internal unnamed_addr global i8 1, section ".msvcjmc"
; CHECK-DAG: @"__B5368CFC_jmc@c" = internal unnamed_addr global i8 1, section ".msvcjmc"
; CHECK-DAG: @"__9596D9CE_a@b@cpp" = internal unnamed_addr global i8 1, section ".msvcjmc"
; CHECK-DAG: @"__656F0D27_x y@cpp" = internal unnamed_addr global i8 1, section ".msvcjmc"
; CHECK-DAG: @{{"?}}__2091DF61_jmc{{"?}} = internal unnamed_addr global i8 1, section ".msvcjmc"
; CHECK-DAG: @"__B60AB3F0_jmc@cpp" = internal unnamed_addr global i8 1, section ".msvcjmc"

; CHECK: declare void @__CheckForDebuggerJustMyCode(ptr noundef)

define void @f0() !dbg !4 {
  ret void
}
define void @f1() !dbg !6 {
  ret void
}
define void @f2() !dbg !8 {
  ret void
}
define void @f3() !dbg !10 {
  ret void
}
define void @f4() !dbg !12 {
  ret void
}
define void @f5() !dbg !14 {
  ret void
}
define void @f6() !dbg !16 {
  ret void
}
define void @f7() !dbg !18 {
  ret void
}
define void @f8() !dbg !20 {
  ret void
}
define void @f9() !dbg !22 {
  ret void
}
define void @f10() !dbg !24 {
  ret void
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!100, !101}
!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "jmc.cpp", directory: "D:\\Projects")
!2 = !DISubroutineType(types: !{null})
!3 = !DIFile(filename: "jmc.cpp", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p1")
!5 = !DIFile(filename: "jmc.cpp", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p2")
!7 = !DIFile(filename: "JMC.CPP", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p1")
!9 = !DIFile(filename: "jmc.c", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p1")
!11 = !DIFile(filename: "a.b.cpp", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p1")
!13 = !DIFile(filename: "x y.cpp", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p3 Space Dir")
!15 = !DIFile(filename: "jmc", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p1")
!17 = !DIFile(filename: "jmc.cpp", directory: "D:/Projects/openrt/tasks/compat/data/jmc_probes/src/p2")
!19 = !DIFile(filename: "..\\\\p1\\\\jmc.cpp", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p2")
!21 = !DIFile(filename: ".\\\\jmc.cpp", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\p1")
!23 = !DIFile(filename: "jmc.cpp", directory: "D:\\\\Projects\\\\openrt\\\\tasks\\\\compat\\\\data\\\\jmc_probes\\\\src\\\\UPPER")
!4 = distinct !DISubprogram(name: "f0", scope: !3, file: !3, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!6 = distinct !DISubprogram(name: "f1", scope: !5, file: !5, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!8 = distinct !DISubprogram(name: "f2", scope: !7, file: !7, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!10 = distinct !DISubprogram(name: "f3", scope: !9, file: !9, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!12 = distinct !DISubprogram(name: "f4", scope: !11, file: !11, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!14 = distinct !DISubprogram(name: "f5", scope: !13, file: !13, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!16 = distinct !DISubprogram(name: "f6", scope: !15, file: !15, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!18 = distinct !DISubprogram(name: "f7", scope: !17, file: !17, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!20 = distinct !DISubprogram(name: "f8", scope: !19, file: !19, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!22 = distinct !DISubprogram(name: "f9", scope: !21, file: !21, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!24 = distinct !DISubprogram(name: "f10", scope: !23, file: !23, line: 1, type: !2, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !{})
!100 = !{i32 2, !"CodeView", i32 1}
!101 = !{i32 2, !"Debug Info Version", i32 3}
