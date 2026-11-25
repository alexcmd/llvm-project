; ML-ERRORREPORT, ML-UNKNOWN-OPTION: ML64.EXE takes /errorReport:{prompt,queue,
; send,none} silently, warns A4018 about an option it does not know and goes
; on, and fails on that warning under /WX.
;
; RUN: llvm-ml -m64 /errorReport:prompt -errorReport:none -filetype=s %s /Fo - 2>&1 | FileCheck %s --check-prefix=OK --implicit-check-not=A4018 --implicit-check-not=unsupported
; RUN: llvm-ml -m64 /errorReport:PROMPT /ERRORREPORT:prompt /errorReport /zzbogus -filetype=s %s /Fo - 2>&1 | FileCheck %s --check-prefixes=WARN,OK
; RUN: not llvm-ml -m64 /WX /zzbogus -filetype=s %s /Fo - 2>&1 | FileCheck %s --check-prefix=WX --implicit-check-not=ret
; The warning goes to stdout, as ML64.EXE writes it.
; RUN: llvm-ml -m64 /zzbogus -zzdash %s /Fo %t.obj 2>%t.err | FileCheck %s --check-prefix=STDOUT
; /W0 and /w silence it, also under /WX.
; RUN: llvm-ml -m64 /WX /W0 /zzbogus -filetype=s %s /Fo - 2>&1 | FileCheck %s --check-prefix=OK --implicit-check-not=A4018
; RUN: llvm-ml -m64 /w /zzbogus -filetype=s %s /Fo - 2>&1 | FileCheck %s --check-prefix=OK --implicit-check-not=A4018
; No source file left: A1017, not a silent success.
; RUN: not llvm-ml -m64 /zzbogus 2>&1 | FileCheck %s --check-prefix=NOSRC
; RUN: not llvm-ml -m64 /c /Fo %t2.obj /zz/nonexistent/t.asm 2>&1 | FileCheck %s --check-prefix=NOSRC
; The listing-file option is known (and unsupported), not unknown.
; RUN: llvm-ml -m64 /Fl%t.lst -filetype=s %s /Fo - 2>&1 | FileCheck %s --check-prefix=OK --implicit-check-not=A4018

; WARN-DAG: MASM : warning A4018:invalid command-line option : /errorReport:PROMPT
; WARN-DAG: MASM : warning A4018:invalid command-line option : /ERRORREPORT:prompt
; WARN-DAG: MASM : warning A4018:invalid command-line option : /errorReport{{$}}
; WARN-DAG: MASM : warning A4018:invalid command-line option : /zzbogus
; WX: MASM : warning A4018:invalid command-line option : /zzbogus
; STDOUT-DAG: MASM : warning A4018:invalid command-line option : /zzbogus
; STDOUT-DAG: MASM : warning A4018:invalid command-line option : -zzdash
; NOSRC: MASM : warning A4018:invalid command-line option : /zz
; NOSRC: MASM : fatal error A1017:missing source filename

.code
foo PROC
  ret
foo ENDP
; OK: ret
END
