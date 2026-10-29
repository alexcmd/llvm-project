; ML64-BUG(cross-seg-jmp) for loop/jrcxz, which have only a rel8 form: under
; /ml64-encoding the branch to a PROC of another segment cannot be encoded
; (warning + error); /ml64-bugcompat writes ML64.EXE's e2 00 / e3 00
; (BUGCOMPAT lines = ML64.EXE 14.51 output for this file).
; RUN: not llvm-ml64 /c /ml64-encoding /Fo %t.m.obj %s 2>&1 | FileCheck %s --check-prefix=ML64
; RUN: llvm-ml64 /c /ml64-bugcompat /Fo %t.b.obj %s 2>&1 | FileCheck %s --check-prefix=WARNBC
; RUN: llvm-objdump -dr %t.b.obj | FileCheck %s --check-prefix=BUGCOMPAT

_TEXT SEGMENT
  db 20h dup (90h)
g PROC
  ret
g ENDP
_TEXT ENDS
X2 SEGMENT READ EXECUTE 'CODE'
h PROC
  loop g
  jrcxz g
  ret
h ENDP
X2 ENDS
END

; ML64:      :17:8: warning: branch to PROC 'g' of another segment: {{.*}}; a rel8 branch cannot reach another section (/ml64-bugcompat: ML64.EXE's bytes)
; ML64:      :17:8: error: unsupported relocation type
; ML64:      :18:9: warning: branch to PROC 'g'
; WARNBC:    :17:8: warning: {{.*}}(/ml64-bugcompat)
; WARNBC:    :18:9: warning: {{.*}}(/ml64-bugcompat)
; BUGCOMPAT-LABEL: <h>:
; BUGCOMPAT-NEXT:  0: e2 00 loop
; BUGCOMPAT-NEXT:  2: e3 00 jrcxz
; BUGCOMPAT-NEXT:  4: c3 retq
