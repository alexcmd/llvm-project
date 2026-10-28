; RUN: llvm-ml -filetype=s %s /Fo /dev/null 2>&1 | FileCheck %s --check-prefix=WARN --implicit-check-not=warning:
; RUN: llvm-ml -filetype=s %s /W1 /Fo /dev/null 2>&1 | FileCheck %s --check-prefix=WARN --implicit-check-not=warning:
; RUN: llvm-ml -filetype=s %s /W3 /Fo /dev/null 2>&1 | FileCheck %s --check-prefix=WARN --implicit-check-not=warning:
; RUN: llvm-ml -filetype=s %s /W0 /W2 /Fo /dev/null 2>&1 | FileCheck %s --check-prefix=WARN --implicit-check-not=warning:

; /W0 and /w disable warnings, even with /WX; the last level wins.
; RUN: llvm-ml -filetype=s %s /W0 /Fo /dev/null 2>&1 | count 0
; RUN: llvm-ml -filetype=s %s /w /Fo /dev/null 2>&1 | count 0
; RUN: llvm-ml -filetype=s %s /W3 /W0 /Fo /dev/null 2>&1 | count 0
; RUN: llvm-ml -filetype=s %s /W0 /WX /Fo /dev/null 2>&1 | count 0

; RUN: not llvm-ml -filetype=s %s /W3 /WX /Fo /dev/null 2>&1 | FileCheck %s --check-prefix=WX

; Unknown levels are ignored with a warning, like ML.EXE's A4008.
; RUN: llvm-ml -filetype=s %s /W4 /Fo /dev/null 2>&1 | FileCheck %s --check-prefixes=BADLEVEL,WARN --implicit-check-not=warning:

; BADLEVEL: warning: invalid command-line option value, default is used: '/W4'
; WARN: :[[# @LINE + 3]]:25: warning: MASM-style hex floats ignore explicit sign
; WX: :[[# @LINE + 2]]:25: error: MASM-style hex floats ignore explicit sign
.data
negative_hexfloat REAL4 -3fa66666r

.code

END
