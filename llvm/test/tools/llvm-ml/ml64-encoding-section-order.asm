; /ml64-encoding: ML64.EXE numbers the _TEXT section 1 and _DATA 2 whatever
; the source opens first; other sections follow in creation order (ML64 lines
; = ML64.EXE 14.51 section table of this file). By default the order is the
; order of first use.
; RUN: llvm-ml64 /c /Fo %t.d.obj %s
; RUN: llvm-objdump -h -s -r %t.d.obj | FileCheck %s --check-prefix=DEFAULT
; RUN: llvm-ml64 /c /ml64-encoding /Fo %t.m.obj %s
; RUN: llvm-objdump -h -s -r %t.m.obj | FileCheck %s --check-prefix=ML64

.data
d1 dq 1
CONST SEGMENT
c1 dq 2
CONST ENDS
.code
f PROC
  mov rax, d1
  ret
f ENDP
END

; DEFAULT:      0 .data         00000008
; DEFAULT-NEXT: 1 {{CONST|\.rdata}} 00000008
; DEFAULT-NEXT: 2 .text{{(\$mn)?}} 00000008

; ML64:      0 .text{{(\$mn)?}} 00000008 {{.*}} TEXT
; ML64-NEXT: 1 .data         00000008 {{.*}} DATA
; ML64-NEXT: 2 {{CONST|\.rdata}} 00000008 {{.*}} DATA
; ML64:      RELOCATION RECORDS FOR [.text{{(\$mn)?}}]:
; ML64:      0000000000000003 IMAGE_REL_AMD64_REL32 d1
; ML64:      Contents of section .text{{(\$mn)?}}:
; ML64-NEXT:  0000 488b0500 000000c3
; ML64:      Contents of section .data:
; ML64-NEXT:  0000 01000000 00000000
; ML64:      Contents of section {{CONST|\.rdata}}:
; ML64-NEXT:  0000 02000000 00000000
