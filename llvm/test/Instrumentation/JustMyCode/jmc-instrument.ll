; RUN: opt -jmc-instrumenter -mtriple=x86_64-pc-windows-msvc  -S < %s | FileCheck %s
; RUN: opt -jmc-instrumenter -mtriple=aarch64-pc-windows-msvc -S < %s | FileCheck %s
; RUN: opt -jmc-instrumenter -mtriple=arm-pc-windows-msvc     -S < %s | FileCheck %s
; RUN: opt -passes=jmc-instrumenter -mtriple=x86_64-pc-windows-msvc  -S < %s | FileCheck %s
; RUN: opt -passes=jmc-instrumenter -mtriple=aarch64-pc-windows-msvc -S < %s | FileCheck %s
; RUN: opt -passes=jmc-instrumenter -mtriple=arm-pc-windows-msvc     -S < %s | FileCheck %s

; CHECK: $__JustMyCode_Default = comdat any

; CHECK: @"__727AB713_x@c" = internal unnamed_addr global i8 1, section ".msvcjmc", align 1, !dbg !{{[0-9]+}}
; CHECK: @"__7DE10BAA_x@c" = internal unnamed_addr global i8 1, section ".msvcjmc", align 1, !dbg !{{[0-9]+}}
; CHECK: @"__412795A9_x@c" = internal unnamed_addr global i8 1, section ".msvcjmc", align 1, !dbg !{{[0-9]+}}

; CHECK: define void @l1() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__727AB713_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @l2() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__727AB713_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @w1() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__7DE10BAA_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @w2() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__7DE10BAA_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @w3() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__7DE10BAA_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @w4() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__7DE10BAA_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @w5() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__412795A9_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @w6() !dbg !{{[0-9]+}} {
; CHECK:   ret void
; CHECK: }

; CHECK: define void @w7() !dbg !{{[0-9]+}} {
; CHECK:   call void @__CheckForDebuggerJustMyCode(ptr noundef @"__412795A9_x@c")
; CHECK:   ret void
; CHECK: }

; CHECK: define void @__JustMyCode_Default(ptr noundef %0) unnamed_addr comdat {
; CHECK:   ret void
; CHECK: }

; CHECK: declare void @__CheckForDebuggerJustMyCode(ptr noundef) unnamed_addr

; CHECK: !llvm.linker.options = !{!{{[0-9]+}}}

; CHECK-DAG: !{{[0-9]+}} = !DIGlobalVariableExpression(var: !{{[0-9]+}}, expr: !DIExpression())
; CHECK-DAG: !{{[0-9]+}} = distinct !DIGlobalVariable(name: "__727AB713_x@c", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, type: !{{[0-9]+}}, isLocal: true, isDefinition: true)
; CHECK-DAG: !{{[0-9]+}} = distinct !DICompileUnit(language: DW_LANG_C99, file: !{{[0-9]+}}, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !{{[0-9]+}}, splitDebugInlining: false, nameTableKind: None)
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "a/x.c", directory: "/tmp")
; CHECK-DAG: !{{[0-9]+}} = !{!{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}}
; CHECK-DAG: !{{[0-9]+}} = !DIGlobalVariableExpression(var: !{{[0-9]+}}, expr: !DIExpression())
; CHECK-DAG: !{{[0-9]+}} = distinct !DIGlobalVariable(name: "__7DE10BAA_x@c", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, type: !{{[0-9]+}}, isLocal: true, isDefinition: true)
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "./x.c", directory: "C:\\\\a\\\\b\\\\")
; CHECK-DAG: !{{[0-9]+}} = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char, flags: DIFlagArtificial)
; CHECK-DAG: !{{[0-9]+}} = !DIGlobalVariableExpression(var: !{{[0-9]+}}, expr: !DIExpression())
; CHECK-DAG: !{{[0-9]+}} = distinct !DIGlobalVariable(name: "__412795A9_x@c", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, type: !{{[0-9]+}}, isLocal: true, isDefinition: true)
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "./b/.\\../b/x.c", directory: "a\\d/p")
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "./b/./../b/x.c", directory: "a/d/p")
; CHECK-DAG: !{{[0-9]+}} = !{i32 2, !"CodeView", i32 1}
; CHECK-DAG: !{{[0-9]+}} = !{i32 2, !"Debug Info Version", i32 3}
; CHECK-DAG: !{{[0-9]+}} = !{!"clang"}
; CHECK-DAG: !{{[0-9]+}} = !{!"/alternatename:__CheckForDebuggerJustMyCode=__JustMyCode_Default"}
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = !DISubroutineType(types: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = !{null}
; CHECK-DAG: !{{[0-9]+}} = !{}
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "x.c", directory: "/tmp/a")
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "./b\\x.c", directory: "C:\\\\a\\\\")
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "./b/x.c", directory: "C:\\\\a\\\\")
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "./b/./../b/x.c", directory: "C:/a")
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "f", scope: !{{[0-9]+}}, file: !{{[0-9]+}}, line: 1, type: !{{[0-9]+}}, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !{{[0-9]+}}, retainedNodes: !{{[0-9]+}})
; CHECK-DAG: !{{[0-9]+}} = !DIFile(filename: "./b/./../b\\x.c", directory: "a/d/p")

; All use the same flag
define void @l1() !dbg !20 {
  ret void
}
define void @l2() !dbg !21 {
  ret void
}

; All use the same flag
define void @w1() !dbg !22 {
  ret void
}
define void @w2() !dbg !23 {
  ret void
}
define void @w3() !dbg !24 {
  ret void
}
define void @w4() !dbg !25 {
  ret void
}

; Test relative paths
define void @w5() !dbg !26 {
  ret void
}
define void @w6() !dbg !27 {
  ret void
}
define void @w7() !dbg !28 {
  ret void
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!17, !18}
!llvm.ident = !{!19}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "a/x.c", directory: "/tmp")
!2 = !DIFile(filename: "x.c", directory: "/tmp/a")
!3 = !DIFile(filename: "./x.c", directory: "C:\\\\a\\\\b\\\\")
!4 = !DIFile(filename: "./b\\x.c", directory: "C:\\\\a\\\\")
!5 = !DIFile(filename: "./b/x.c", directory: "C:\\\\a\\\\")
!6 = !DIFile(filename: "./b/./../b/x.c", directory: "C:/a")
!7 = !DIFile(filename: "./b/.\\../b/x.c", directory: "a\\d/p")
!8 = !DIFile(filename: "./b/./../b/x.c", directory: "a/d/p")
!9 = !DIFile(filename: "./b/./../b\\x.c", directory: "a/d/p")
!17 = !{i32 2, !"CodeView", i32 1}
!18 = !{i32 2, !"Debug Info Version", i32 3}
!19 = !{!"clang"}
!20 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!21 = distinct !DISubprogram(name: "f", scope: !2, file: !2, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!22 = distinct !DISubprogram(name: "f", scope: !3, file: !3, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!23 = distinct !DISubprogram(name: "f", scope: !4, file: !4, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!24 = distinct !DISubprogram(name: "f", scope: !5, file: !5, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!25 = distinct !DISubprogram(name: "f", scope: !6, file: !6, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!26 = distinct !DISubprogram(name: "f", scope: !7, file: !7, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!27 = distinct !DISubprogram(name: "f", scope: !8, file: !8, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!28 = distinct !DISubprogram(name: "f", scope: !9, file: !9, line: 1, type: !31, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!31 = !DISubroutineType(types: !32)
!32 = !{null}
!33 = !{}
