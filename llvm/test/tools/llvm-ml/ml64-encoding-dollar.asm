; /ml64-encoding: '$' in code is the location counter, a LABEL-class symbol
; '$$<offset as 6 hex digits>' that RIP-relative operands relocate against
; (ML64 lines = ML64.EXE 14.51 output for this file). Without it '$' stays an
; undefined symbol named '$' (DEFAULT).
; RUN: llvm-ml -m64 /c /Fo %t.default.obj %s
; RUN: llvm-objdump -dr -M intel %t.default.obj | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: llvm-ml -m64 /c /ml64-encoding /Fo %t.ml64.obj %s
; RUN: llvm-objdump -dr -M intel %t.ml64.obj | FileCheck %s --check-prefixes=CHECK,ML64
; RUN: llvm-readobj --symbols %t.ml64.obj | FileCheck %s --check-prefix=SYMS

_TEXT SEGMENT
f PROC
  lea rax, $
  nop
  lea rcx, $
  lea rdx, $+5
  ret
f ENDP
_TEXT ENDS
END

; CHECK-LABEL: <f>:
; DEFAULT-NEXT:  48 8d 04 25 00 00 00 00  lea rax, [0x0]
; DEFAULT-NEXT:  IMAGE_REL_AMD64_ADDR32 $
; ML64-NEXT:     48 8d 05 00 00 00 00     lea rax, [rip]
; ML64-NEXT:     IMAGE_REL_AMD64_REL32 $$000000
; ML64-NEXT:     90                       nop
; ML64-EMPTY:
; ML64-NEXT:     <$$000008>:
; ML64-NEXT:     48 8d 0d 00 00 00 00     lea rcx, [rip]
; ML64-NEXT:     IMAGE_REL_AMD64_REL32 $$000008
; ML64-EMPTY:
; ML64-NEXT:     <$$00000F>:
; ML64-NEXT:     48 8d 15 05 00 00 00     lea rdx, [rip + 0x5]
; ML64-NEXT:     IMAGE_REL_AMD64_REL32 $$00000F

; SYMS:      Name: $$000000
; SYMS-NEXT: Value: 0
; SYMS-NEXT: Section: .text
; SYMS-NEXT: BaseType: Null
; SYMS-NEXT: ComplexType: Null
; SYMS-NEXT: StorageClass: Label (0x6)
; SYMS:      Name: $$000008
; SYMS-NEXT: Value: 8
; SYMS:      Name: $$00000F
; SYMS-NEXT: Value: 15
