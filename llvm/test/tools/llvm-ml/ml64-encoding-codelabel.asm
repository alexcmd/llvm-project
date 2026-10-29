; /ml64-encoding: an untyped 'name:' label as the source of mov r32/r64 is its
; address, as in ML64.EXE (brackets do not matter): movabs + ADDR64, or
; b8 + ADDR32; 'qword ptr label' and typed data names stay loads (ML64 lines =
; ML64.EXE 14.51 output for this file). ML64.EXE rejects mov ax/push/cmp/add
; with such a label. By default the label is a memory operand.
; RUN: llvm-ml64 /c /Fo %t.d.obj %s
; RUN: llvm-objdump -dr -M intel %t.d.obj | FileCheck %s --check-prefix=DEFAULT
; RUN: llvm-ml64 /c /ml64-encoding /Fo %t.m.obj %s
; RUN: llvm-objdump -dr -M intel %t.m.obj | FileCheck %s --check-prefix=ML64

_DATA SEGMENT
dvar dq 0
dlab:
  dq 0
_DATA ENDS
_TEXT SEGMENT
lab:
  nop
f PROC
  mov rax, lab
  mov r9, lab
  mov eax, lab
  mov rax, lab+4
  mov rax, [lab]
  mov rax, qword ptr lab
  mov rax, dlab
  mov rax, dvar
  mov rax, OFFSET lab
  lea rax, lab
  ret
f ENDP
_TEXT ENDS
END

; DEFAULT-LABEL: <f>:
; DEFAULT-NEXT:  48 8b 04 25 00 00 00 00 mov rax, qword ptr [0x0]
; DEFAULT-NEXT:  IMAGE_REL_AMD64_ADDR32 lab

; ML64-LABEL: <f>:
; ML64-NEXT:  48 b8 00 00 00 00 00 00 00 00 movabs rax, 0x0
; ML64-NEXT:  IMAGE_REL_AMD64_ADDR64 lab
; ML64-NEXT:  49 b9 00 00 00 00 00 00 00 00 movabs r9, 0x0
; ML64-NEXT:  IMAGE_REL_AMD64_ADDR64 lab
; ML64-NEXT:  b8 00 00 00 00                mov eax, 0x0
; ML64-NEXT:  IMAGE_REL_AMD64_ADDR32 lab
; ML64-NEXT:  48 b8 04 00 00 00 00 00 00 00 movabs rax, 0x4
; ML64-NEXT:  IMAGE_REL_AMD64_ADDR64 lab
; ML64-NEXT:  48 b8 00 00 00 00 00 00 00 00 movabs rax, 0x0
; ML64-NEXT:  IMAGE_REL_AMD64_ADDR64 lab
; ML64-NEXT:  48 8b 05 00 00 00 00          mov rax, qword ptr [rip]
; ML64-NEXT:  IMAGE_REL_AMD64_REL32 lab
; ML64-NEXT:  48 b8 00 00 00 00 00 00 00 00 movabs rax, 0x0
; ML64-NEXT:  IMAGE_REL_AMD64_ADDR64 dlab
; ML64-NEXT:  48 8b 05 00 00 00 00          mov rax, qword ptr [rip]
; ML64-NEXT:  IMAGE_REL_AMD64_REL32 dvar
; ML64-NEXT:  48 b8 00 00 00 00 00 00 00 00 movabs rax, 0x0
; ML64-NEXT:  IMAGE_REL_AMD64_ADDR64 lab
; ML64-NEXT:  48 8d 05 00 00 00 00          lea rax, [rip]
; ML64-NEXT:  IMAGE_REL_AMD64_REL32 lab
