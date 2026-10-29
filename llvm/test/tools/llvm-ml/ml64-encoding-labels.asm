; /ml64-encoding: a local symbol other than a PROC is in the symbol table only
; when a relocation references it; 'name:' labels have storage class LABEL (6),
; data names stay STATIC, PUBLIC ones EXTERNAL (ML64 lines = ML64.EXE 14.51
; symbols of this file). By default every label is a STATIC symbol.
; RUN: llvm-ml64 /c /Fo %t.d.obj %s
; RUN: llvm-objdump -t %t.d.obj | FileCheck %s --check-prefix=DEFAULT
; RUN: llvm-ml64 /c /ml64-encoding /Fo %t.m.obj %s
; RUN: llvm-objdump -t %t.m.obj | FileCheck %s --check-prefix=ML64 \
; RUN:   --implicit-check-not=dv_unused --implicit-check-not=dtab \
; RUN:   --implicit-check-not=dcol_unused --implicit-check-not=c_out \
; RUN:   --implicit-check-not=c_none --implicit-check-not=c_jmp \
; RUN:   --implicit-check-not=c_fwd

PUBLIC publ
_DATA SEGMENT
dv_used dq 0
dv_unused dq 0
dtab dq c_tab
dcol_used:
  dq 0
dcol_unused:
  dq 0
_DATA ENDS
_TEXT SEGMENT
c_out:
  nop
c_tab:
  nop
publ:
  nop
f PROC
c_none:
  nop
c_jmp:
  jmp c_jmp
  jmp c_fwd
c_fwd:
  lea rax, c_lea
c_lea:
  mov rax, dv_used
  lea rax, dcol_used
  ret
f ENDP
g PROC
  ret
g ENDP
_TEXT ENDS
END

; DEFAULT-DAG: (scl   3) (nx 0) 0x00000008 dv_unused
; DEFAULT-DAG: (scl   3) (nx 0) 0x00000003 c_none
; DEFAULT-DAG: (scl   3) (nx 0) 0x00000001 c_tab

; ML64-DAG: (scl   2) (nx 0) 0x00000002 publ
; ML64-DAG: (scl   6) (nx 0) 0x00000018 dcol_used
; ML64-DAG: (scl   3) (nx 0) 0x00000000 dv_used
; ML64-DAG: (scl   6) (nx 0) 0x00000001 c_tab
; ML64-DAG: (ty  20)(scl   2) (nx 0) 0x00000003 f
; ML64-DAG: (scl   6) (nx 0) 0x0000000f c_lea
; ML64-DAG: (ty  20)(scl   2) (nx 0) 0x0000001e g
