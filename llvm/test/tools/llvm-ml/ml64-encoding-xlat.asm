; /ml64-encoding: 'xlat byte ptr seg:[(e)bx]' keeps the segment override and a
; 32-bit base's address-size prefix (67, then the segment), as ML64.EXE does
; (ML64 lines = ML64.EXE 14.51 output for this file); by default the operand
; only gives the size and every form is d7.
; RUN: llvm-ml -m64 /c /Fo %t.default.obj %s 2>&1
; RUN: llvm-objdump -d -M intel %t.default.obj | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: llvm-ml -m64 /c /ml64-encoding /Fo %t.ml64.obj %s 2>&1
; RUN: llvm-objdump -d -M intel %t.ml64.obj | FileCheck %s --check-prefixes=CHECK,ML64

_TEXT SEGMENT
f PROC
; CHECK-LABEL: <f>:
  xlat byte ptr gs:[rbx]
; DEFAULT-NEXT: d7       xlatb
; ML64-NEXT:    65 d7    xlatb
  xlat byte ptr fs:[rbx]
; DEFAULT-NEXT: d7       xlatb
; ML64-NEXT:    64 d7    xlatb
  xlat byte ptr [rbx]
; CHECK-NEXT:   d7       xlatb
  xlatb
; CHECK-NEXT:   d7       xlatb
  xlat byte ptr gs:[ebx]
; DEFAULT-NEXT: d7       xlatb
; ML64-NEXT:    67 65 d7 addr32 xlatb
  xlat byte ptr [ebx]
; DEFAULT-NEXT: d7       xlatb
; ML64-NEXT:    67 d7    addr32 xlatb
  ret
f ENDP
_TEXT ENDS
END
