; RUN: llvm-ml -m64 %s /Fo - | llvm-readobj --section-headers - | FileCheck %s

; ML64.EXE section names for the predefined segments, their '$'-suffixed
; (grouped) forms and the simplified segment directives; the predefined names
; match case-insensitively. Names and characteristics match ML64.EXE 14.51.

_TEXT SEGMENT
 nop
_TEXT ENDS
_TEXT$21 SEGMENT
 nop
_TEXT$21 ENDS
_TEXT$ab SEGMENT
 nop
_TEXT$ab ENDS
_DATA SEGMENT
 db 1
_DATA ENDS
_DATA$x SEGMENT
 db 2
_DATA$x ENDS
CONST SEGMENT
 db 3
CONST ENDS
CONST$x SEGMENT
 db 4
CONST$x ENDS
_BSS SEGMENT
 db ?
_BSS ENDS
_BSS$x SEGMENT
 db ?
_BSS$x ENDS
FOO$x SEGMENT
 db 5
FOO$x ENDS
_text SEGMENT
 nop
_text ENDS
_TEXTX SEGMENT
 nop
_TEXTX ENDS
.code
 nop
.data
 db 6
.const
 db 7
.data?
 db ?

; _TEXT, _text and .code share .text$mn.
; CHECK-LABEL: Name: .text$mn (
; CHECK-NEXT:  VirtualSize:
; CHECK-NEXT:  VirtualAddress:
; CHECK-NEXT:  RawDataSize: 3
; CHECK:       Characteristics [ (0x60500020)
; CHECK-LABEL: Name: .text$mn$21 (
; CHECK:       Characteristics [ (0x60500020)
; CHECK-LABEL: Name: .text$mn$ab (
; CHECK:       Characteristics [ (0x60500020)
; _DATA and .data share .data.
; CHECK-LABEL: Name: .data (
; CHECK-NEXT:  VirtualSize:
; CHECK-NEXT:  VirtualAddress:
; CHECK-NEXT:  RawDataSize: 2
; CHECK:       Characteristics [ (0xC0500040)
; CHECK-LABEL: Name: .data$x (
; CHECK:       Characteristics [ (0xC0500040)
; CONST and .const share the read-only .rdata.
; CHECK-LABEL: Name: .rdata (
; CHECK-NEXT:  VirtualSize:
; CHECK-NEXT:  VirtualAddress:
; CHECK-NEXT:  RawDataSize: 2
; CHECK:       Characteristics [ (0x40500040)
; CHECK-LABEL: Name: .rdata$x (
; CHECK:       Characteristics [ (0x40500040)
; _BSS and .data? share the uninitialized .bss.
; CHECK-LABEL: Name: .bss (
; CHECK:       Characteristics [ (0xC0500080)
; CHECK-LABEL: Name: .bss$x (
; CHECK:       Characteristics [ (0xC0500080)
; Other names are kept as is.
; CHECK-LABEL: Name: FOO$x (
; CHECK:       Characteristics [ (0xC0500040)
; CHECK-LABEL: Name: _TEXTX (
; CHECK:       Characteristics [ (0xC0500040)

END
