# An explicit {load}/{store} keeps its ModRM direction under
# -x86-masm-encoding, which otherwise gives reg,reg ALU/MOV ML64.EXE's
# "reg, r/m" form (xor eax, ecx = 33 c1).
# RUN: llvm-mc -triple x86_64-pc-windows-msvc -show-encoding %s | FileCheck %s --check-prefixes=CHECK,DEFAULT
# RUN: llvm-mc -triple x86_64-pc-windows-msvc -show-encoding -x86-masm-encoding %s | FileCheck %s --check-prefixes=CHECK,ML64

# CHECK:   xorl %ecx, %eax # encoding: [0x31,0xc8]
{store} xor %ecx, %eax
# CHECK:   xorl %ecx, %eax # encoding: [0x33,0xc1]
{load} xor %ecx, %eax
# DEFAULT: xorl %ecx, %eax # encoding: [0x31,0xc8]
# ML64:    xorl %ecx, %eax # encoding: [0x33,0xc1]
xor %ecx, %eax
# CHECK:   movq %r8, %r9 # encoding: [0x4d,0x89,0xc1]
{store} mov %r8, %r9
# CHECK:   movaps %xmm1, %xmm0 # encoding: [0x0f,0x29,0xc8]
{store} movaps %xmm1, %xmm0
