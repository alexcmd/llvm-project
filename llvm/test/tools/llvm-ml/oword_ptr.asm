; MASM's OWORD PTR is a 16-byte memory operand (cmpxchg16b oword ptr [rcx]).
; Expected lines: ML64.EXE 14.51 output for this file.
; RUN: llvm-ml64 /nologo /c /Fo %t.obj %s
; RUN: llvm-objdump -dr -M intel %t.obj | FileCheck %s

.data
x qword 0, 0

.code
cmpxchg16b oword ptr [rcx]
; CHECK:      48 0f c7 09                  cmpxchg16b xmmword ptr [rcx]
cmpxchg16b OWORD PTR [rsp+8]
; CHECK-NEXT: 48 0f c7 4c 24 08            cmpxchg16b xmmword ptr [rsp + 0x8]
lock cmpxchg16b oword ptr [rdx+rax*8]
; CHECK-NEXT: f0                           lock
; CHECK-NEXT: 48 0f c7 0c c2               cmpxchg16b xmmword ptr [rdx + 8*rax]
lock cmpxchg16b oword ptr x
; CHECK-NEXT: f0                           lock
; CHECK-NEXT: 48 0f c7 0d 00 00 00 00      cmpxchg16b xmmword ptr [rip]
; CHECK-NEXT: IMAGE_REL_AMD64_REL32 x
movdqa xmm0, oword ptr [rcx]
; CHECK-NEXT: 66 0f 6f 01                  movdqa xmm0, xmmword ptr [rcx]

end
