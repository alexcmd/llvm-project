; ML64.EXE: A2161/A2163/A2164 non-benign structure redefinition.
; RUN: not llvm-ml64 -filetype=s %s /Fo /dev/null 2>&1 | FileCheck %s --implicit-check-not=error:

T struct
  x dword ?
T ends
T2 struct
  x dword ?
T2 ends

S struct
  a dword ?
  b qword ?
S ends

; field renamed
S struct
  a dword ?
  c qword ?
S ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'S'

; field size
S struct
  a dword ?
  b dword ?
S ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'S'

; field kind (real4 vs dword)
S struct
  a real4 ?
  b qword ?
S ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'S'

; element count
S struct
  a dword 2 dup (?)
  b qword ?
S ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'S'

; field order
S struct
  b dword ?
  a qword ?
S ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'S'

; extra field
S struct
  a dword ?
  b qword ?
  c byte ?
S ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'S'

; struct vs union
S union
  a dword ?
  b qword ?
S ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'S'

; nested field of another (same-layout) type
N struct
  f T <>
N ends
N struct
  f T2 <>
N ends
; CHECK: :[[#@LINE-1]]:1: error: non-benign structure redefinition: 'N'

end
