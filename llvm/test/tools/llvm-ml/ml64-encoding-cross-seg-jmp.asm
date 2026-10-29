; ML64-BUG(cross-seg-jmp): ML64.EXE 14.51 sizes a jmp/jcc to a PROC of another
; code segment as if both were in one segment (B, T: offsets in their own
; segments): for T-(B+2) in [-128, 136] (jmp) / [-128, 134] (jcc), and always
; for 'jmp short'/loop/jrcxz, it writes the short form with displacement 0 and
; no relocation (it continues at the next instruction); else the correct near
; form ('call' is right). All branches here are in that range.
; /ml64-encoding warns at each such branch and keeps the correct e9/0f 8x +
; REL32; /ml64-bugcompat (implies /ml64-encoding) writes ML64.EXE's bytes
; (BUGCOMPAT lines = ML64.EXE output for this file). Without /ml64-encoding
; there is no warning.
; RUN: llvm-ml64 /c /Fo %t.d.obj %s 2>&1 | count 0
; RUN: llvm-ml64 /c /ml64-encoding /Fo %t.m.obj %s 2>&1 | FileCheck %s --check-prefix=WARN
; RUN: llvm-objdump -dr -M intel %t.m.obj | FileCheck %s --check-prefix=ML64
; RUN: llvm-ml64 /c /ml64-bugcompat /Fo %t.b.obj %s 2>&1 | FileCheck %s --check-prefix=WARNBC
; RUN: llvm-objdump -dr -M intel %t.b.obj | FileCheck %s --check-prefix=BUGCOMPAT

X0 SEGMENT READ EXECUTE 'CODE'
b0 PROC
  db 30h dup (90h)
  ret
b0 ENDP
X0 ENDS
_TEXT SEGMENT
f PROC
  nop
  ret
f ENDP
  db 40h dup (90h)
g PROC
  ret
g ENDP
_TEXT ENDS
X2 SEGMENT READ EXECUTE 'CODE'
  db 10h dup (0CCh)
h PROC
  jmp g
  jmp f
  db 20h dup (0CCh)
  jmp g
  jz g
  jnz f
  jmp short g
  jmp b0
  call g
  jmp h2
  ret
h ENDP
h2 PROC
  ret
h2 ENDP
X2 ENDS
X3 SEGMENT READ EXECUTE 'CODE'
  db 7 dup (0CCh)
k PROC
  jmp g
  jmp b0
  jmp h
  ret
k ENDP
X3 ENDS
END

; WARN:      :36:7: warning: branch to PROC 'g' of another segment: ML64.EXE encodes it as a short jump with displacement 0 and no relocation (it continues at the next instruction) when the distance taken within one segment fits (jmp -128..136, jcc -128..134; loop/jrcxz/jmp short always); emitting the branch to the PROC (/ml64-bugcompat: ML64.EXE's bytes)
; WARN:      :37:7: warning: branch to PROC 'f' of another segment
; WARN:      :39:7: warning: branch to PROC 'g'
; WARN:      :40:6: warning: branch to PROC 'g'
; WARN:      :41:7: warning: branch to PROC 'f'
; WARN:      :42:13: warning: branch to PROC 'g'
; WARN:      :43:7: warning: branch to PROC 'b0'
; WARN-NOT:  :44:{{.*}}warning
; WARN:      :55:7: warning: branch to PROC 'g'
; WARN:      :56:7: warning: branch to PROC 'b0'
; WARN:      :57:7: warning: branch to PROC 'h'
; WARNBC:    :36:7: warning: {{.*}}; emitting ML64.EXE's bytes (/ml64-bugcompat)

; ML64-LABEL: <h>:
; ML64-NEXT:  e9 00 00 00 00 jmp
; ML64-NEXT:  IMAGE_REL_AMD64_REL32 g
; ML64-NEXT:  e9 00 00 00 00 jmp
; ML64-NEXT:  IMAGE_REL_AMD64_REL32 f
; ML64:       0f 84 00 00 00 00 je
; ML64-NEXT:  IMAGE_REL_AMD64_REL32 g
; ML64:       e8 00 00 00 00 call
; ML64-NEXT:  IMAGE_REL_AMD64_REL32 g

; BUGCOMPAT-LABEL: <h>:
; BUGCOMPAT-NEXT:  10: eb 00 jmp
; BUGCOMPAT-NEXT:  12: eb 00 jmp
; BUGCOMPAT:       34: eb 00 jmp
; BUGCOMPAT-NEXT:  36: 74 00 je
; BUGCOMPAT-NEXT:  38: 75 00 jne
; BUGCOMPAT-NEXT:  3a: eb 00 jmp
; BUGCOMPAT-NEXT:  3c: eb 00 jmp
; BUGCOMPAT-NEXT:  3e: e8 00 00 00 00 call
; BUGCOMPAT-NEXT:  IMAGE_REL_AMD64_REL32 g
; BUGCOMPAT-NEXT:  43: eb 01 jmp
; BUGCOMPAT:       <k>:
; BUGCOMPAT-NEXT:  7: eb 00 jmp
; BUGCOMPAT-NEXT:  9: eb 00 jmp
; BUGCOMPAT-NEXT:  b: eb 00 jmp
; BUGCOMPAT-NEXT:  d: c3 ret
