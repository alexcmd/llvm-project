; /ml64-encoding unwind tables (PROC FRAME) as ML64.EXE writes them (ML64 lines
; = ML64.EXE 14.51 output for this file); without it llvm-ml keeps its own
; (DEFAULT): a code section's .pdata right after it, one .xdata (8-aligned,
; created at the first .endprolog), 4-byte UNWIND_INFO without codes,
; SET_FPREG op info = frame register, .pdata relocations against the function
; and '$xdatasym' (static, .xdata+0).
; RUN: llvm-ml -m64 /c /Fo %t.default.obj %s
; RUN: llvm-readobj --sections --section-data --section-relocations --symbols %t.default.obj | FileCheck %s --check-prefix=DEFAULT
; RUN: llvm-ml -m64 /c /ml64-encoding /Fo %t.ml64.obj %s
; RUN: llvm-readobj --sections --section-data --section-relocations --symbols %t.ml64.obj | FileCheck %s --check-prefix=ML64

_TEXT SEGMENT
f0 PROC FRAME
  push rbx
  .pushreg rbx
  sub rsp, 20h
  .allocstack 20h
  .endprolog
  add rsp, 20h
  pop rbx
  ret
f0 ENDP
f1 PROC FRAME
  .endprolog
  ret
f1 ENDP
leaf PROC
  ret
leaf ENDP
_TEXT ENDS
OTHER SEGMENT READ EXECUTE 'CODE'
g0 PROC FRAME
  push rbp
  .pushreg rbp
  mov rbp, rsp
  .setframe rbp, 0
  .endprolog
  pop rbp
  ret
g0 ENDP
g1 PROC FRAME
  sub rsp, 8
  .allocstack 8
  .endprolog
  add rsp, 8
  ret
g1 ENDP
OTHER ENDS
END

; DEFAULT:      Name: .text
; DEFAULT:      Name: .xdata
; DEFAULT:        IMAGE_SCN_ALIGN_4BYTES
; DEFAULT:        0000: 01050200 05320130 01000000 00000000
; DEFAULT:      Name: OTHER
; DEFAULT:      Name: .xdata
; DEFAULT:        0000: 01040205 04030150 01040100 04020000
; DEFAULT:      Name: .pdata
; DEFAULT:        0x0 IMAGE_REL_AMD64_ADDR32NB .text
; DEFAULT-NEXT:   0x4 IMAGE_REL_AMD64_ADDR32NB .text
; DEFAULT-NEXT:   0x8 IMAGE_REL_AMD64_ADDR32NB .xdata
; DEFAULT:        0000: 00000000 0B000000 00000000 0B000000
; DEFAULT-NEXT:   0010: 0C000000 08000000
; DEFAULT:      Name: .pdata
; DEFAULT:        0x0 IMAGE_REL_AMD64_ADDR32NB OTHER
; DEFAULT:        0000: 00000000 06000000 00000000 06000000
; DEFAULT-NEXT:   0010: 0F000000 08000000
; DEFAULT-NOT:  $xdatasym

; ML64:      Name: .text
; ML64:      Name: .pdata
; ML64:        IMAGE_SCN_ALIGN_4BYTES
; ML64:        0x0 IMAGE_REL_AMD64_ADDR32NB f0
; ML64-NEXT:   0x4 IMAGE_REL_AMD64_ADDR32NB f0
; ML64-NEXT:   0x8 IMAGE_REL_AMD64_ADDR32NB $xdatasym
; ML64-NEXT:   0xC IMAGE_REL_AMD64_ADDR32NB f1
; ML64-NEXT:   0x10 IMAGE_REL_AMD64_ADDR32NB f1
; ML64-NEXT:   0x14 IMAGE_REL_AMD64_ADDR32NB $xdatasym
; ML64:        0000: 00000000 0B000000 00000000 00000000
; ML64-NEXT:   0010: 01000000 08000000
; ML64:      Name: .xdata
; ML64:        IMAGE_SCN_ALIGN_8BYTES
; ML64:        0000: 01050200 05320130 01000000 01040205
; ML64-NEXT:   0010: 04530150 01040100 04020000
; ML64:      Name: OTHER
; ML64:      Name: .pdata
; ML64:        0x0 IMAGE_REL_AMD64_ADDR32NB g0
; ML64-NEXT:   0x4 IMAGE_REL_AMD64_ADDR32NB g0
; ML64-NEXT:   0x8 IMAGE_REL_AMD64_ADDR32NB $xdatasym
; ML64-NEXT:   0xC IMAGE_REL_AMD64_ADDR32NB g1
; ML64-NEXT:   0x10 IMAGE_REL_AMD64_ADDR32NB g1
; ML64-NEXT:   0x14 IMAGE_REL_AMD64_ADDR32NB $xdatasym
; ML64:        0000: 00000000 06000000 0C000000 00000000
; ML64-NEXT:   0010: 09000000 14000000
; ML64:      Name: $xdatasym
; ML64-NEXT:   Value: 0
; ML64-NEXT:   Section: .xdata
; ML64-NEXT:   BaseType: Null
; ML64-NEXT:   ComplexType: Null
; ML64-NEXT:   StorageClass: Static
