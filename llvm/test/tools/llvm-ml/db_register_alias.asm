; MASM has no db0..db15 debug-register aliases: db8 is an ordinary name
; (it was taken for DR8: "invalid operand for instruction"). ML64.EXE 14.51:
; cmp db8, 80h = 80 3d <rel32> 80, relocated against db8.
; RUN: llvm-ml64 /c /Fo %t.obj %s
; RUN: llvm-objdump -dr -M intel %t.obj | FileCheck %s

_DATA SEGMENT
db8 DB 0
db15 DB 0
_DATA ENDS
_TEXT SEGMENT
f PROC
  cmp db8, 80h
; CHECK:      80 3d {{.*}} cmp byte ptr [rip
; CHECK-NEXT: IMAGE_REL_AMD64_REL32{{(_1)?}} db8
  cmp byte ptr db15, 0
; CHECK-NEXT: 80 3d {{.*}} cmp byte ptr [rip
; CHECK-NEXT: IMAGE_REL_AMD64_REL32{{(_1)?}} db15
  ret
f ENDP
_TEXT ENDS
END
