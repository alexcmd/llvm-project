// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 \
// RUN:   -show-encoding %s | FileCheck %s
// RUN: llvm-mc -triple x86_64-unknown-unknown -show-encoding -defsym ATT=1 %s \
// RUN:   | FileCheck %s --check-prefix=ATT

// Intel syntax (v)movd with a qword memory operand: the REX.W / VEX.W1
// 66 0f 6e / 7e form, like (v)movd with a 64-bit GPR. (v)movq keeps
// f3 0f 7e / 66 0f d6.
.intel_syntax noprefix
// CHECK: vmovq xmm2, qword ptr [rax + r9]
// CHECK-SAME: encoding: [0xc4,0xa1,0xf9,0x6e,0x14,0x08]
vmovd xmm2, qword ptr [rax + r9]
// CHECK: vmovq xmm2, qword ptr [rax + r9]
// CHECK-SAME: encoding: [0xc4,0xa1,0x7a,0x7e,0x14,0x08]
vmovq xmm2, qword ptr [rax + r9]
// CHECK: vmovq qword ptr [rax], xmm2
// CHECK-SAME: encoding: [0xc4,0xe1,0xf9,0x7e,0x10]
vmovd qword ptr [rax], xmm2
// CHECK: vmovq qword ptr [rax], xmm2
// CHECK-SAME: encoding: [0xc5,0xf9,0xd6,0x10]
vmovq qword ptr [rax], xmm2
// CHECK: movq xmm2, qword ptr [rax + r9]
// CHECK-SAME: encoding: [0x66,0x4a,0x0f,0x6e,0x14,0x08]
movd xmm2, qword ptr [rax + r9]
// CHECK: movq qword ptr [rax], xmm2
// CHECK-SAME: encoding: [0x66,0x48,0x0f,0x7e,0x10]
movd qword ptr [rax], xmm2
// CHECK: vmovd xmm2, dword ptr [rax]
// CHECK-SAME: encoding: [0xc5,0xf9,0x6e,0x10]
vmovd xmm2, dword ptr [rax]
// CHECK: movd xmm2, dword ptr [rax]
// CHECK-SAME: encoding: [0x66,0x0f,0x6e,0x10]
movd xmm2, dword ptr [rax]

.ifdef ATT
.att_syntax
// AT&T: an unsized memory operand stays 32-bit.
// ATT: movd (%rcx), %xmm3
// ATT-SAME: encoding: [0x66,0x0f,0x6e,0x19]
movd (%rcx), %xmm3
// ATT: vmovd (%rcx), %xmm3
// ATT-SAME: encoding: [0xc5,0xf9,0x6e,0x19]
vmovd (%rcx), %xmm3
.endif
