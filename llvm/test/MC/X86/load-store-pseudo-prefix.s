// RUN: llvm-mc -triple x86_64-unknown-unknown -show-encoding %s | FileCheck %s

// GNU as {load} / {store}: pick the MRMSrcReg (reg <- r/m) or MRMDestReg
// (r/m <- reg) encoding of a reg,reg instruction that has both.

// CHECK: xorl %eax, %eax
// CHECK-SAME: encoding: [0x31,0xc0]
xorl %eax, %eax
// CHECK: xorl %eax, %eax
// CHECK-SAME: encoding: [0x33,0xc0]
{load} xorl %eax, %eax
// CHECK: xorl %eax, %eax
// CHECK-SAME: encoding: [0x31,0xc0]
{store} xorl %eax, %eax
// CHECK: subq %rcx, %rdx
// CHECK-SAME: encoding: [0x48,0x2b,0xd1]
{load} subq %rcx, %rdx
// CHECK: orb %r8b, %dl
// CHECK-SAME: encoding: [0x41,0x0a,0xd0]
{load} orb %r8b, %dl
// CHECK: addw %r9w, %r10w
// CHECK-SAME: encoding: [0x66,0x45,0x03,0xd1]
{load} addw %r9w, %r10w
// CHECK: adcl %ecx, %edx
// CHECK-SAME: encoding: [0x13,0xd1]
{load} adcl %ecx, %edx
// CHECK: sbbl %eax, %eax
// CHECK-SAME: encoding: [0x1b,0xc0]
{load} sbbl %eax, %eax
// CHECK: andl %r11d, %edi
// CHECK-SAME: encoding: [0x41,0x23,0xfb]
{load} andl %r11d, %edi
// CHECK: cmpq %rdx, %r9
// CHECK-SAME: encoding: [0x4c,0x3b,0xca]
{load} cmpq %rdx, %r9
// CHECK: movl %ecx, %eax
// CHECK-SAME: encoding: [0x8b,0xc1]
{load} movl %ecx, %eax
// CHECK: movq %rcx, %rax
// CHECK-SAME: encoding: [0x48,0x89,0xc8]
{store} movq %rcx, %rax

// The SSE/AVX moves default to the load form; {store} picks 0f 29 / 0f 11 ...
// CHECK: movaps %xmm1, %xmm0
// CHECK-SAME: encoding: [0x0f,0x28,0xc1]
{load} movaps %xmm1, %xmm0
// CHECK: movaps %xmm1, %xmm0
// CHECK-SAME: encoding: [0x0f,0x29,0xc8]
{store} movaps %xmm1, %xmm0
// CHECK: movups %xmm1, %xmm0
// CHECK-SAME: encoding: [0x0f,0x11,0xc8]
{store} movups %xmm1, %xmm0
// CHECK: vmovaps %xmm1, %xmm0
// CHECK-SAME: encoding: [0xc5,0xf8,0x29,0xc8]
{store} vmovaps %xmm1, %xmm0
// ... and the form wins over the VEX2-friendly commutation.
// CHECK: vmovaps %xmm8, %xmm0
// CHECK-SAME: encoding: [0xc5,0x78,0x29,0xc0]
vmovaps %xmm8, %xmm0
// CHECK: vmovaps %xmm8, %xmm0
// CHECK-SAME: encoding: [0xc4,0xc1,0x78,0x28,0xc0]
{load} vmovaps %xmm8, %xmm0

// No other direction (no FOO/FOO_REV pair): the default encoding stays.
// CHECK: testl %ecx, %eax
// CHECK-SAME: encoding: [0x85,0xc8]
{load} testl %ecx, %eax
// CHECK: addl $1, %eax
// CHECK-SAME: encoding: [0x83,0xc0,0x01]
{load} addl $1, %eax
// CHECK: xorl (%rcx), %eax
// CHECK-SAME: encoding: [0x33,0x01]
{store} xorl (%rcx), %eax

.intel_syntax noprefix
// CHECK: xorl %eax, %eax
// CHECK-SAME: encoding: [0x33,0xc0]
{load} xor eax, eax
// CHECK: subq %rcx, %rdx
// CHECK-SAME: encoding: [0x48,0x2b,0xd1]
{load} sub rdx, rcx
// CHECK: movq %rcx, %rax
// CHECK-SAME: encoding: [0x48,0x8b,0xc1]
{load} mov rax, rcx
