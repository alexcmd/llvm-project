; ML64-BUG(cross-seg-jmp) range: ML64.EXE 14.51 miscompiles a jmp/jcc to a PROC
; of another segment only when T-(B+2) (B, T: offsets in their own segments)
; is in [-128, 136] for jmp, [-128, 134] for jcc; just outside it encodes the
; correct near branch. /ml64-bugcompat reproduces both (BUGCOMPAT lines =
; ML64.EXE output for this file); the warning names only the in-range ones.
; RUN: llvm-ml64 /c /ml64-bugcompat /Fo %t.obj %s 2>&1 | FileCheck %s --check-prefix=WARN --implicit-check-not=warning:
; RUN: llvm-objdump -dr %t.obj | FileCheck %s --check-prefix=BUGCOMPAT

X2 SEGMENT READ EXECUTE 'CODE'
  db 74 dup (90h)
tn128 PROC
  ret
tn128 ENDP
tn129 PROC
  ret
tn129 ENDP
  db 62 dup (90h)
t136 PROC
  ret
t136 ENDP
  db 2 dup (90h)
t137 PROC
  ret
t137 ENDP
  db 1 dup (90h)
z134 PROC
  ret
z134 ENDP
  db 2 dup (90h)
z135 PROC
  ret
z135 ENDP
X2 ENDS
_TEXT SEGMENT
f PROC
  jmp t136
  jmp t137
  jz z134
  jz z135
  db 185 dup (90h)
  jmp tn128
  jmp tn129
  ret
f ENDP
_TEXT ENDS
END

; WARN: warning: branch to PROC 't136' of another segment
; WARN: warning: branch to PROC 'z134' of another segment
; WARN: warning: branch to PROC 'tn128' of another segment

; BUGCOMPAT-LABEL: <f>:
; BUGCOMPAT-NEXT:  0: eb 00 jmp
; BUGCOMPAT-NEXT:  2: e9 00 00 00 00 jmp
; BUGCOMPAT-NEXT:  IMAGE_REL_AMD64_REL32 t137
; BUGCOMPAT-NEXT:  7: 74 00 je
; BUGCOMPAT-NEXT:  9: 0f 84 00 00 00 00 je
; BUGCOMPAT-NEXT:  IMAGE_REL_AMD64_REL32 z135
; BUGCOMPAT:       c8: eb 00 jmp
; BUGCOMPAT-NEXT:  ca: e9 00 00 00 00 jmp
; BUGCOMPAT-NEXT:  IMAGE_REL_AMD64_REL32 tn129
; BUGCOMPAT-NEXT:  cf: c3 retq
