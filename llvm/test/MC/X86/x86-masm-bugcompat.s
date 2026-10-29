# ML64-BUG(cross-seg-jmp): -x86-masm-encoding warns at a jmp/jcc to a function
# of another section and keeps the correct rel32 branch; -x86-masm-bugcompat
# writes ML64.EXE's short jump with displacement 0 and no relocation (see
# tools/llvm-ml/ml64-encoding-cross-seg-*.asm for the ML64.EXE bytes).
# RUN: llvm-mc -triple x86_64-pc-windows-msvc -filetype=obj %s -o %t.d.obj 2>&1 | count 0
# RUN: llvm-mc -triple x86_64-pc-windows-msvc -filetype=obj -x86-masm-encoding %s -o %t.m.obj 2>&1 | FileCheck %s --check-prefix=WARN
# RUN: llvm-objdump -dr %t.m.obj | FileCheck %s --check-prefix=ML64
# RUN: llvm-mc -triple x86_64-pc-windows-msvc -filetype=obj -x86-masm-encoding -x86-masm-bugcompat %s -o %t.b.obj 2>&1 | FileCheck %s --check-prefix=WARN
# RUN: llvm-objdump -dr %t.b.obj | FileCheck %s --check-prefix=BUGCOMPAT

.def g
.scl 2
.type 32
.endef
.section .text$a,"xr"
g:
  ret

.section .text$b,"xr"
.globl h
h:
  jmp g
  je g
  call g
  ret

# WARN: warning: branch to PROC 'g' of another segment
# WARN: warning: branch to PROC 'g' of another segment
# WARN-NOT: warning

# ML64:      <h>:
# ML64-NEXT: e9 00 00 00 00
# ML64-NEXT: IMAGE_REL_AMD64_REL32 g
# ML64-NEXT: 0f 84 00 00 00 00
# ML64-NEXT: IMAGE_REL_AMD64_REL32 g
# ML64-NEXT: e8 00 00 00 00
# ML64-NEXT: IMAGE_REL_AMD64_REL32 g

# BUGCOMPAT:      <h>:
# BUGCOMPAT-NEXT: 0: eb 00
# BUGCOMPAT-NEXT: 2: 74 00
# BUGCOMPAT-NEXT: 4: e8 00 00 00 00
# BUGCOMPAT-NEXT: IMAGE_REL_AMD64_REL32 g
