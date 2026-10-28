; RUN: llvm-ml -m64 %s /Fo %t.obj
; RUN: llvm-readobj -r %t.obj | FileCheck %s
; RUN: llvm-objdump -s -j '.rdata$00' %t.obj | FileCheck %s --check-prefix=DATA
; RUN: not llvm-ml -m64 -filetype=s %s /DBAD /Fo - 2>&1 | FileCheck %s --check-prefix=ERR

; IMAGEREL <symbol> is the image-relative address (RVA) of the symbol:
; IMAGE_REL_AMD64_ADDR32NB, as an immediate, as a memory displacement (with or
; without parentheses) and in DD data. Expected values match ML64.EXE 14.51.

extrn ext:byte

_TEXT$21 SEGMENT
foo PROC
 mov eax, IMAGEREL foo
 mov eax, IMAGEREL ext + 8
 mov r9d, [(IMAGEREL tbl) + r10 + r8*4]
 mov r9d, [IMAGEREL tbl + r10 + r8*4]
 mov r9d, DWORD PTR [r10 + IMAGEREL tbl]
 mov r9d, [r10 + r8*4 + IMAGEREL tbl + 4]
; LEA of an OFFSET expression is RIP-relative, like LEA of the symbol itself.
 lea r10, OFFSET ext
 lea rax, ext
 push IMAGEREL foo
 ret
foo ENDP
_TEXT$21 ENDS

_RDATA SEGMENT READ PARA ALIAS('.rdata$00') 'DATA'
tbl DD IMAGEREL foo
    DD IMAGEREL foo + 3
    DD IMAGEREL ext
    DD 2 DUP (IMAGEREL foo)
    DD IMAGEREL tbl
_RDATA ENDS

IFDEF BAD
.data
; ERR: :[[# @LINE + 1]]:13: error: expected symbol after IMAGEREL
DD IMAGEREL 5
ENDIF

; CHECK:      Section ({{[0-9]+}}) .text$mn$21 {
; CHECK-NEXT:   0x1 IMAGE_REL_AMD64_ADDR32NB foo
; CHECK-NEXT:   0x6 IMAGE_REL_AMD64_ADDR32NB ext
; CHECK-NEXT:   0xE IMAGE_REL_AMD64_ADDR32NB tbl
; CHECK-NEXT:   0x16 IMAGE_REL_AMD64_ADDR32NB tbl
; CHECK-NEXT:   0x1D IMAGE_REL_AMD64_ADDR32NB tbl
; CHECK-NEXT:   0x25 IMAGE_REL_AMD64_ADDR32NB tbl
; CHECK-NEXT:   0x2C IMAGE_REL_AMD64_REL32 ext
; CHECK-NEXT:   0x33 IMAGE_REL_AMD64_REL32 ext
; CHECK-NEXT:   0x38 IMAGE_REL_AMD64_ADDR32NB foo
; CHECK-NEXT: }
; CHECK-NEXT: Section ({{[0-9]+}}) .rdata$00 {
; CHECK-NEXT:   0x0 IMAGE_REL_AMD64_ADDR32NB foo
; CHECK-NEXT:   0x4 IMAGE_REL_AMD64_ADDR32NB foo
; CHECK-NEXT:   0x8 IMAGE_REL_AMD64_ADDR32NB ext
; CHECK-NEXT:   0xC IMAGE_REL_AMD64_ADDR32NB foo
; CHECK-NEXT:   0x10 IMAGE_REL_AMD64_ADDR32NB foo
; CHECK-NEXT:   0x14 IMAGE_REL_AMD64_ADDR32NB tbl
; CHECK-NEXT: }

; Addends are stored in place.
; DATA:      0000 00000000 03000000 00000000 00000000
; DATA-NEXT: 0010 00000000 00000000

END
