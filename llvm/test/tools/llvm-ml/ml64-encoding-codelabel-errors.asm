; /ml64-encoding: the forms ML64.EXE 14.51 rejects for an untyped 'name:' label
; used as an address constant, with its error numbers (A2070 invalid
; instruction operands, A2084 constant value too large, A2024 invalid operand
; size). Without /ml64-encoding the label is a memory operand.
; RUN: not llvm-ml64 /c /ml64-encoding /Fo %t.obj %s 2>&1 | FileCheck %s --implicit-check-not=error:

_TEXT SEGMENT
lab:
  nop
f PROC
  mov qword ptr [rcx], lab
; CHECK: :[[#@LINE-1]]:3: error: constant value too large: label 'lab' is a 64-bit address constant here (ML64.EXE A2084)
  test rax, lab
; CHECK: :[[#@LINE-1]]:3: error: constant value too large: label 'lab' is a 64-bit address constant here (ML64.EXE A2084)
  mov al, lab
; CHECK: :[[#@LINE-1]]:3: error: invalid instruction operands: label 'lab' is an address constant here (ML64.EXE A2070)
  imul rax, rcx, lab
; CHECK: :[[#@LINE-1]]:3: error: invalid instruction operands: label 'lab' is an address constant here (ML64.EXE A2070)
  sub rcx, lab
; CHECK: :[[#@LINE-1]]:3: error: constant value too large: label 'lab' is a 64-bit address constant here (ML64.EXE A2084)
  push lab
; CHECK: :[[#@LINE-1]]:3: error: invalid instruction operands: label 'lab' is an address constant here (ML64.EXE A2070)
  mov ax, lab
; CHECK: :[[#@LINE-1]]:3: error: invalid operand size for instruction: label 'lab' is an address constant here (ML64.EXE A2024)
  cmp rax, lab
; CHECK: :[[#@LINE-1]]:3: error: constant value too large: label 'lab' is a 64-bit address constant here (ML64.EXE A2084)
  add rax, lab
; CHECK: :[[#@LINE-1]]:3: error: constant value too large: label 'lab' is a 64-bit address constant here (ML64.EXE A2084)
  ret
f ENDP
_TEXT ENDS
END
