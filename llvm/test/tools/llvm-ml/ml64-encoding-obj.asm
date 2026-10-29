; /ml64-encoding object output: ALIGN nops, relocations and branch resolution
; of ML64.EXE (ML64 lines = ML64.EXE 14.51 bytes and relocations); without it
; llvm-ml keeps its own (DEFAULT).
; RUN: llvm-ml -m64 /c /Fo %t.default.obj %s
; RUN: llvm-objdump -dr -M intel %t.default.obj | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: llvm-ml -m64 /c /ml64-encoding /DINDEXED /Fo %t.ml64.obj %s
; RUN: llvm-objdump -dr -M intel %t.ml64.obj | FileCheck %s --check-prefixes=CHECK,ML64

EXTERN ext:PROC

_DATA SEGMENT
bvar DB 0
wvar DW 0
dvar DD 0
qvar DQ 0
_DATA ENDS

; ALIGN in code: 15-byte pieces; 10..15 bytes are (n-8) x 66 + 0f 1f 84 00..
; (LLVM: the cs-prefixed form 66 .. 2e 0f 1f 84 00..).
ALN SEGMENT ALIGN(64) 'CODE'
a PROC
; CHECK-LABEL: <a>:
  int 3
  ALIGN 16
; DEFAULT-NEXT: cc
; DEFAULT-NEXT: 66 66 66 66 66 66 2e 0f 1f 84 00 00 00 00 00
; ML64-NEXT:    cc
; ML64-NEXT:    66 66 66 66 66 66 66 0f 1f 84 00 00 00 00 00
  db 6 dup (0CCh)
  ALIGN 16
; ML64:         66 66 0f 1f 84 00 00 00 00 00 nop
  db 7 dup (0CCh)
  ALIGN 16
; CHECK:        66 0f 1f 84 00 00 00 00 00 nop
  db 47 dup (0CCh)
  ALIGN 64
; DEFAULT:      66 66 66 66 66 66 2e 0f 1f 84 00 00 00 00 00 nop
; DEFAULT-NEXT: 66 66 66 66 66 66 2e 0f 1f 84 00 00 00 00 00 nop
; DEFAULT-NEXT: 0f 1f 00 nop
; ML64:         66 66 66 66 66 66 66 0f 1f 84 00 00 00 00 00 nop
; ML64-NEXT:    66 66 66 66 66 66 66 0f 1f 84 00 00 00 00 00 nop
; ML64-NEXT:    0f 1f 00 nop
  ret
a ENDP
ALN ENDS

_TEXT SEGMENT
tdat DD 1
f PROC
; CHECK-LABEL: <f>:
; RIP-relative operand + immediate: REL32_<imm size> with no immediate bias.
  cmp bvar, 80h
; DEFAULT:      80 3d ff ff ff ff 80
; DEFAULT-NEXT: IMAGE_REL_AMD64_REL32 bvar
; ML64:         80 3d 00 00 00 00 80
; ML64-NEXT:    IMAGE_REL_AMD64_REL32_1 bvar
  cmp byte ptr [bvar+3], 7
; DEFAULT:      80 3d 02 00 00 00 07
; ML64:         80 3d 03 00 00 00 07
; ML64-NEXT:    IMAGE_REL_AMD64_REL32_1 bvar
  cmp word ptr [wvar], 700h
; ML64:         66 81 3d 00 00 00 00 00 07
; ML64-NEXT:    IMAGE_REL_AMD64_REL32_2 wvar
  mov qword ptr [qvar], 1
; DEFAULT:      48 c7 05 fc ff ff ff 01 00 00 00
; ML64:         48 c7 05 00 00 00 00 01 00 00 00
; ML64-NEXT:    IMAGE_REL_AMD64_REL32_4 qvar
  lea rcx, [qvar-4]
; CHECK:        48 8d 0d fc ff ff ff
; CHECK-NEXT:   IMAGE_REL_AMD64_REL32 qvar

; mov r64, OFFSET sym: ADDR64.
  mov rax, OFFSET qvar
; DEFAULT:      48 c7 c0 00 00 00 00
; DEFAULT-NEXT: IMAGE_REL_AMD64_ADDR32 qvar
; ML64:         48 b8 00 00 00 00 00 00 00 00
; ML64-NEXT:    IMAGE_REL_AMD64_ADDR64 qvar

; A RIP-relative operand keeps its relocation inside its own section.
  mov eax, tdat
; DEFAULT:      8b 05 {{.. .. .. ..}} mov eax, dword ptr [rip
; DEFAULT-NOT:  IMAGE_REL
; ML64:         8b 05 00 00 00 00
; ML64-NEXT:    IMAGE_REL_AMD64_REL32 tdat

; Branches to a function of the same section are resolved.
  call g
; DEFAULT:      e8 00 00 00 00
; DEFAULT-NEXT: IMAGE_REL_AMD64_REL32 g
; ML64:         e8 {{.. .. .. ..}} call
; ML64-NOT:     IMAGE_REL
  jmp g
; DEFAULT:      e9 00 00 00 00
; DEFAULT-NEXT: IMAGE_REL_AMD64_REL32 g
; ML64:         eb {{..}} jmp
; ML64-NOT:     IMAGE_REL
  call ext
; CHECK:        e8 00 00 00 00
; CHECK-NEXT:   IMAGE_REL_AMD64_REL32 ext

; '$' is the location counter.
  jmp $
; DEFAULT:      e9 00 00 00 00
; DEFAULT-NEXT: IMAGE_REL_AMD64_REL32 $
; ML64:         eb fe

IFDEF INDEXED
; [label+index*s]: absolute disp32 (ADDR32), not RIP-relative.
  jmp qword ptr [qvar+rax*8]
; ML64:         ff 24 c5 00 00 00 00
; ML64-NEXT:    IMAGE_REL_AMD64_ADDR32 qvar
ENDIF
  ret
f ENDP
g PROC
  ret
g ENDP
_TEXT ENDS
END
