; /ml64-encoding: an untyped 'name:' label without PTR is an address constant
; for every source operand ML64.EXE accepts - 32-bit forms take imm32 +
; ADDR32 (eax gets the short accumulator form), mov r64 imm64 + ADDR64 - also
; when the label is defined after the use (ML64.EXE reads twice; llvm-ml
; decides at layout); a forward data name or 'qword ptr' stays a load. ML64 lines = ML64.EXE
; 14.51 output for this file.
; RUN: llvm-ml64 /c /ml64-encoding /Fo %t.obj %s
; RUN: llvm-objdump -dr -M intel %t.obj | FileCheck %s

_TEXT SEGMENT
lab:
  nop
f PROC
  add eax, lab
  cmp eax, lab
  and eax, lab
  mov ecx, lab+8
  push qword ptr lab
  mov dword ptr [rcx], lab
  mov rax, fwd
  mov eax, fwd
  mov rcx, fwd+4
  mov rax, dfwd
  mov rax, qword ptr fwd
  ret
f ENDP
fwd:
  ret
_TEXT ENDS
_DATA SEGMENT
dfwd dq 0
_DATA ENDS
END

; CHECK-LABEL: <f>:
; CHECK-NEXT: 05 00 00 00 00                add eax, 0x0
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 lab
; CHECK-NEXT: 3d 00 00 00 00                cmp eax, 0x0
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 lab
; CHECK-NEXT: 25 00 00 00 00                and eax, 0x0
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 lab
; CHECK-NEXT: b9 08 00 00 00                mov ecx, 0x8
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 lab
; CHECK-NEXT: ff 35 00 00 00 00             push qword ptr [rip]
; CHECK-NEXT: IMAGE_REL_AMD64_REL32 lab
; CHECK-NEXT: c7 01 00 00 00 00             mov dword ptr [rcx], 0x0
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 lab
; CHECK-NEXT: 48 b8 00 00 00 00 00 00 00 00 movabs rax, 0x0
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR64 fwd
; CHECK-NEXT: b8 00 00 00 00                mov eax, 0x0
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 fwd
; CHECK-NEXT: 48 b9 04 00 00 00 00 00 00 00 movabs rcx, 0x4
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR64 fwd
; CHECK-NEXT: 48 8b 05 00 00 00 00          mov rax, qword ptr [rip]
; CHECK-NEXT: IMAGE_REL_AMD64_REL32 dfwd
; CHECK-NEXT: 48 8b 05 00 00 00 00          mov rax, qword ptr [rip]
; CHECK-NEXT: IMAGE_REL_AMD64_REL32 fwd
; CHECK-NEXT: c3                            ret
; CHECK:      <fwd>:
