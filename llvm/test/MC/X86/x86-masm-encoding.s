# -x86-masm-encoding (llvm-ml /ml64-encoding): ML64.EXE operand forms and
# prefix order; tools/llvm-ml/ml64-encoding*.asm have the full rule set.
# RUN: llvm-mc -triple x86_64-pc-windows-msvc -show-encoding %s | FileCheck %s --check-prefix=DEFAULT
# RUN: llvm-mc -triple x86_64-pc-windows-msvc -show-encoding -x86-masm-encoding %s | FileCheck %s --check-prefix=ML64

.intel_syntax noprefix
# DEFAULT: encoding: [0x48,0x29,0xca]
# ML64:    encoding: [0x48,0x2b,0xd1]
sub rdx, rcx
# DEFAULT: encoding: [0x48,0x85,0xc8]
# ML64:    encoding: [0x48,0x85,0xc1]
test rax, rcx
# DEFAULT: encoding: [0xc5,0xb1,0xfe,0xc1]
# ML64:    encoding: [0xc4,0xc1,0x71,0xfe,0xc1]
vpaddd xmm0, xmm1, xmm9
# DEFAULT: encoding: [0x65,0x66,0xf0,0x83,0x01,0x01]
# ML64:    encoding: [0xf0,0x66,0x65,0x83,0x01,0x01]
lock add word ptr gs:[rcx], 1
