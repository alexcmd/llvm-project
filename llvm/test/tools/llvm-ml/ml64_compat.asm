; RUN: llvm-ml -m64 %s /Fo %t.obj
; RUN: llvm-objdump -s -j .data %t.obj | FileCheck %s --check-prefix=DATA
; RUN: llvm-readobj --symbols %t.obj | FileCheck %s --check-prefix=SYM
; RUN: llvm-readobj --unwind %t.obj | FileCheck %s --check-prefix=UNWIND

; Constructs used by the Windows SDK macamd64.inc macros and the MSVC CRT x64
; MASM sources (memcpy.asm, memset.asm); expected values match ML64.EXE 14.51.

; SUBTTL is a listing directive (synonym of SUBTITLE).
        subttl  "memcpy"

_CS_ EQU <_TEXT$21>

.data
; IFDIF / IFIDN expand a text macro operand (NESTED_ENTRY in macamd64.inc:
; ifdif <Section>, _CurrentSection_).
ifdif <_TEXT$21>, _CS_
 db 0EEh
else
 db 1
endif
ifidn <_TEXT$21>, _CS_
 db 2
endif
; The operands of an IFDIF inside a skipped block are not evaluated.
ifdef UNDEFINED_SYMBOL
ifdif <x>, UNDEFINED_SYMBOL
 db 0EEh
endif
 db 0EEh
endif
 db 3
; DATA: 0000 010203

.code
 public foo_alias
foo PROC
 nop
; 'label::' is a label with global scope.
lbl::
 ret
foo ENDP
; '=' with a relocatable value defines an alias (memmove = memcpy).
foo_alias = foo

; SYM:      Name: foo_alias
; SYM-NEXT: Value: 0
; SYM-NEXT: Section: .text$mn (
; SYM:      StorageClass: External (0x2)

; Without /unwindv3, an epilog opened by .beginepilog runs to ENDP; it is
; encoded as an UNWIND_INFO version 2 epilog code at the end of the function.
bar PROC FRAME
 push rdi
 .pushreg rdi
 push rsi
 .pushreg rsi
 .endprolog
 nop
 .beginepilog
 pop rsi
 pop rdi
 ret
bar ENDP

; UNWIND:      UnwindInfo {
; UNWIND-NEXT:   Version: 2
; UNWIND-NEXT:   Flags [ (0x0)
; UNWIND-NEXT:   ]
; UNWIND-NEXT:   PrologSize: 2
; UNWIND-NEXT:   FrameRegister: -
; UNWIND-NEXT:   FrameOffset: -
; UNWIND-NEXT:   UnwindCodeCount: 4
; UNWIND-NEXT:   UnwindCodes [
; UNWIND-NEXT:     0x03: EPILOG atend=yes, length=0x3
; UNWIND-NEXT:     0x00: EPILOG padding
; UNWIND-NEXT:     0x02: PUSH_NONVOL reg=RSI
; UNWIND-NEXT:     0x01: PUSH_NONVOL reg=RDI
; UNWIND-NEXT:   ]

END
