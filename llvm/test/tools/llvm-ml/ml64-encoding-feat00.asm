; /ml64-encoding: @feat.00 as ML64.EXE writes it - a static symbol, 0x10 always
; set, 0x4000 with /guard:ehcont (the last /guard:ehcont[-] wins; /guard:cf is
; accepted without effect). Values: ML64.EXE 14.51 objects of this file.
; By default llvm-ml keeps its global @feat.00 = 2.
; RUN: llvm-ml64 /c /Fo %t.d.obj %s
; RUN: llvm-readobj --symbols %t.d.obj | FileCheck %s --check-prefix=DEFAULT
; RUN: llvm-ml64 /c /ml64-encoding /Fo %t.m.obj %s
; RUN: llvm-readobj --symbols %t.m.obj | FileCheck %s --check-prefix=ML64
; RUN: llvm-ml64 /c /ml64-encoding /guard:cf /guard:ehcont /Fo %t.e.obj %s
; RUN: llvm-readobj --symbols %t.e.obj | FileCheck %s --check-prefix=EHCONT
; RUN: llvm-ml64 /c /ml64-encoding /guard:ehcont /guard:ehcont- /Fo %t.n.obj %s
; RUN: llvm-readobj --symbols %t.n.obj | FileCheck %s --check-prefix=ML64
; RUN: llvm-ml64 /c /guard:ehcont /Fo %t.de.obj %s
; RUN: llvm-readobj --symbols %t.de.obj | FileCheck %s --check-prefix=DEFAULT-EHCONT

_TEXT SEGMENT
f PROC
  ret
f ENDP
_TEXT ENDS
END

; DEFAULT:      Name: @feat.00
; DEFAULT-NEXT: Value: 2
; DEFAULT-NEXT: Section: IMAGE_SYM_ABSOLUTE
; DEFAULT-NEXT: BaseType: Null
; DEFAULT-NEXT: ComplexType: Null
; DEFAULT-NEXT: StorageClass: External

; ML64:      Name: @feat.00
; ML64-NEXT: Value: 16
; ML64-NEXT: Section: IMAGE_SYM_ABSOLUTE
; ML64-NEXT: BaseType: Null
; ML64-NEXT: ComplexType: Null
; ML64-NEXT: StorageClass: Static

; EHCONT:      Name: @feat.00
; EHCONT-NEXT: Value: 16400
; EHCONT:      StorageClass: Static

; DEFAULT-EHCONT:      Name: @feat.00
; DEFAULT-EHCONT-NEXT: Value: 16386
; DEFAULT-EHCONT:      StorageClass: External
