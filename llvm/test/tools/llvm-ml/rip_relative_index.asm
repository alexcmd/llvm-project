; [sym+index*s] has no RIP-relative form: it is an absolute disp32 + SIB with
; an ADDR32 relocation (previously the index was dropped, or an assertion).
; Expected lines: ML64.EXE 14.51 output for this file.
; RUN: llvm-ml64 /nologo /c /Fo %t.obj %s
; RUN: llvm-objdump -dr --no-print-imm-hex -M intel %t.obj | FileCheck %s

.data
tbl qword 1, 2, 3

.code
f proc
  mov rax, [tbl+rax*8]
; CHECK:      48 8b 04 c5 00 00 00 00   mov rax, qword ptr [8*rax]
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 tbl
  mov rax, qword ptr [tbl+rcx*8]
; CHECK-NEXT: 48 8b 04 cd 00 00 00 00   mov rax, qword ptr [8*rcx]
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 tbl
  jmp qword ptr [tbl+rax*8]
; CHECK-NEXT: ff 24 c5 00 00 00 00      jmp qword ptr [8*rax]
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 tbl
  lea rdx, [tbl+rcx*4]
; CHECK-NEXT: 48 8d 14 8d 00 00 00 00   lea rdx, [4*rcx]
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 tbl
  mov rax, tbl[rcx*8]
; CHECK-NEXT: 48 8b 04 cd 00 00 00 00   mov rax, qword ptr [8*rcx]
; CHECK-NEXT: IMAGE_REL_AMD64_ADDR32 tbl
  mov rax, [tbl+8]
; CHECK-NEXT: 48 8b 05 08 00 00 00      mov rax, qword ptr [rip + 8]
; CHECK-NEXT: IMAGE_REL_AMD64_REL32 tbl
  ret
f endp
end
