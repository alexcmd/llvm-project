; ML64.EXE accepts a benign STRUCT/UNION redefinition (same field names, kinds,
; sizes and counts in the same order) and keeps the first definition: its
; alignment and initializers. Values: ML64.EXE 14.51 output for this file.
; RUN: llvm-ml64 -filetype=s %s /Fo - | FileCheck %s

S struct 8
  a dword 5
  b qword ?
S ends

S struct
  a dword 7
  b qword ?
S ends

s STRUCT
  A SDWORD 9
  B QWORD ?
s ENDS

U union
  x dword ?
  y byte ?
U ends

U union
  x dword ?
  y byte ?
U ends

.data
v S <>
; CHECK-LABEL: v:
; CHECK-NEXT: .long 5
; CHECK-NEXT: .zero 4
; CHECK-NEXT: .quad 0

.code
t1:
mov eax, S.b
mov eax, sizeof S
mov eax, sizeof U
; CHECK-LABEL: t1:
; CHECK-NEXT: mov eax, 8
; CHECK-NEXT: mov eax, 16
; CHECK-NEXT: mov eax, 4

end
