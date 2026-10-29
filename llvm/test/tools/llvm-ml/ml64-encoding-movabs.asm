; /ml64-encoding: MASM constants keep their sign - a literal written positive
; that does not fit a sign-extended imm32 (0FFFFFFFFFFFFFFFFh = 2^64-1) is
; mov r64, imm64 (48 b8), a negative one (-1) the 7-byte 48 c7 c0 form
; (ML64 lines = ML64.EXE 14.51 output for this file); by default both are -1.
; RUN: llvm-ml -m64 /c /Fo %t.default.obj %s
; RUN: llvm-objdump -d -M intel %t.default.obj | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: llvm-ml -m64 /c /ml64-encoding /Fo %t.ml64.obj %s
; RUN: llvm-objdump -d -M intel %t.ml64.obj | FileCheck %s --check-prefixes=CHECK,ML64

_TEXT SEGMENT
f PROC
; CHECK-LABEL: <f>:
  mov rax, 0FFFFFFFFFFFFFFFFh
; DEFAULT-NEXT: 48 c7 c0 ff ff ff ff          mov rax, -0x1
; ML64-NEXT:    48 b8 ff ff ff ff ff ff ff ff movabs rax, -0x1
  mov rcx, -1
; CHECK-NEXT:   48 c7 c1 ff ff ff ff          mov rcx, -0x1
  mov r9, 0FFFFFFFFFFFFFFFFh
; DEFAULT-NEXT: 49 c7 c1 ff ff ff ff          mov r9, -0x1
; ML64-NEXT:    49 b9 ff ff ff ff ff ff ff ff movabs r9, -0x1
  mov rax, 0FFFFFFFF80000000h
; DEFAULT-NEXT: 48 c7 c0 00 00 00 80          mov rax, -0x80000000
; ML64-NEXT:    48 b8 00 00 00 80 ff ff ff ff movabs rax, -0x80000000
  mov rax, 0FFFFFFFFh
; CHECK-NEXT:   48 b8 ff ff ff ff 00 00 00 00 movabs rax, 0xffffffff
  mov rax, -80000000h
; CHECK-NEXT:   48 c7 c0 00 00 00 80          mov rax, -0x80000000
  mov rax, 7FFFFFFFh
; CHECK-NEXT:   48 c7 c0 ff ff ff 7f          mov rax, 0x7fffffff
  mov eax, 0FFFFFFFFh
; CHECK-NEXT:   b8 ff ff ff ff                mov eax, 0xffffffff
  ret
f ENDP
_TEXT ENDS
END
