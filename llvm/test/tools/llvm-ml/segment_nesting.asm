; Segments nest: ENDS returns to the enclosing open segment, so code after a
; nested 'CONST SEGMENT ... CONST ENDS' continues in the code segment
; (it used to land in CONST). Bytes: ML64.EXE 14.51 output for this file.
; RUN: llvm-ml64 /c /Fo %t.obj %s
; RUN: llvm-objdump -d -s %t.obj | FileCheck %s

_TEXT SEGMENT
f PROC
  nop
CONST SEGMENT
c1 dq 1
CONST ENDS
  ret
f ENDP
_TEXT ENDS
_DATA SEGMENT
d1 dq 2
_DATA ENDS
END

; CHECK:      Contents of section {{CONST|\.rdata}}:
; CHECK-NEXT:  0000 01000000 00000000
; CHECK:      Contents of section {{_DATA|\.data}}:
; CHECK-NEXT:  0000 02000000 00000000
; CHECK:      <f>:
; CHECK-NEXT: 90 nop
; CHECK-NEXT: c3 ret
