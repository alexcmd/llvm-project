; /ml64-encoding: instruction forms and prefix order of ML64.EXE (ML64 lines =
; ML64.EXE 14.51 bytes); without it llvm-ml keeps its own encodings (DEFAULT).
; RUN: llvm-ml -m64 -filetype=s -show-encoding %s /Fo - | FileCheck %s --check-prefixes=CHECK,DEFAULT
; RUN: llvm-ml -m64 -filetype=s -show-encoding /ml64-encoding %s /Fo - | FileCheck %s --check-prefixes=CHECK,ML64
; RUN: llvm-ml -m64 -filetype=s -show-encoding -ml64-encoding %s /Fo - | FileCheck %s --check-prefixes=CHECK,ML64

_DATA SEGMENT
bvar DB 0
wvar DW 0
dvar DD 0
qvar DQ 0
_DATA ENDS

_TEXT SEGMENT
f PROC
; reg,reg ALU/MOV: ML64 takes the "reg, r/m" opcode (02/03, 0a/0b, ... 8a/8b).
; CHECK-LABEL: f:
  add rax, rcx
; DEFAULT: encoding: [0x48,0x01,0xc8]
; ML64:    encoding: [0x48,0x03,0xc1]
  or ecx, edx
; DEFAULT: encoding: [0x09,0xd1]
; ML64:    encoding: [0x0b,0xca]
  adc r9, r8
; DEFAULT: encoding: [0x4d,0x11,0xc1]
; ML64:    encoding: [0x4d,0x13,0xc8]
  sbb eax, eax
; DEFAULT: encoding: [0x19,0xc0]
; ML64:    encoding: [0x1b,0xc0]
  and al, ah
; DEFAULT: encoding: [0x20,0xe0]
; ML64:    encoding: [0x22,0xc4]
  sub rdx, rcx
; DEFAULT: encoding: [0x48,0x29,0xca]
; ML64:    encoding: [0x48,0x2b,0xd1]
  xor r8d, r9d
; DEFAULT: encoding: [0x45,0x31,0xc8]
; ML64:    encoding: [0x45,0x33,0xc1]
  cmp cx, r10w
; DEFAULT: encoding: [0x66,0x44,0x39,0xd1]
; ML64:    encoding: [0x66,0x41,0x3b,0xca]
  mov r9, r8
; DEFAULT: encoding: [0x4d,0x89,0xc1]
; ML64:    encoding: [0x4d,0x8b,0xc8]
  mov al, r15b
; DEFAULT: encoding: [0x44,0x88,0xf8]
; ML64:    encoding: [0x41,0x8a,0xc7]

; TEST r, r: the first operand goes to ModRM.reg.
  test rax, rcx
; DEFAULT: encoding: [0x48,0x85,0xc8]
; ML64:    encoding: [0x48,0x85,0xc1]
  test r8b, cl
; DEFAULT: encoding: [0x41,0x84,0xc8]
; ML64:    encoding: [0x44,0x84,0xc1]

; xchg rax,rax keeps its REX.W; xchg eax,eax is the one-byte nop.
  xchg rax, rax
; DEFAULT: encoding: [0x90]
; ML64:    encoding: [0x48,0x90]
  xchg eax, eax
; DEFAULT: encoding: [0x87,0xc0]
; ML64:    encoding: [0x90]
  xchg rcx, rax
; CHECK:   encoding: [0x48,0x91]

; VEX: no operand commutation / _REV form for a 2-byte VEX.
  vpaddd xmm0, xmm1, xmm9
; DEFAULT: encoding: [0xc5,0xb1,0xfe,0xc1]
; ML64:    encoding: [0xc4,0xc1,0x71,0xfe,0xc1]
  vmovaps xmm0, xmm15
; DEFAULT: encoding: [0xc5,0x78,0x29,0xf8]
; ML64:    encoding: [0xc4,0xc1,0x78,0x28,0xc7]
  vmovss xmm0, xmm0, xmm8
; DEFAULT: encoding: [0xc5,0x7a,0x11,0xc0]
; ML64:    encoding: [0xc4,0xc1,0x7a,0x10,0xc0]
  vcmpps xmm0, xmm0, xmm9, 0
; DEFAULT: encoding: [0xc5,0xb0,0xc2,0xc0,0x00]
; ML64:    encoding: [0xc4,0xc1,0x78,0xc2,0xc1,0x00]
  vmovq xmm0, xmm8
; DEFAULT: encoding: [0xc5,0x79,0xd6,0xc0]
; ML64:    encoding: [0xc4,0xc1,0x7a,0x7e,0xc0]

; ret 0, segment register push/pop and moves.
  ret 0
; DEFAULT: encoding: [0xc2,0x00,0x00]
; ML64:    encoding: [0xc3]
  ret 8
; CHECK:   encoding: [0xc2,0x08,0x00]
  push fs
; DEFAULT: encoding: [0x66,0x0f,0xa0]
; ML64:    encoding: [0x0f,0xa0]
  pop gs
; DEFAULT: encoding: [0x66,0x0f,0xa9]
; ML64:    encoding: [0x0f,0xa9]
  mov ax, ds
; DEFAULT: encoding: [0x8c,0xd8]
; ML64:    encoding: [0x66,0x8c,0xd8]
  mov fs, ax
; CHECK:   encoding: [0x66,0x8e,0xe0]

; mov r64, OFFSET sym: movabs with a 64-bit address.
  mov rax, OFFSET qvar
; DEFAULT: encoding: [0x48,0xc7,0xc0,A,A,A,A]
; DEFAULT-NEXT: value: qvar, kind: reloc_signed_4byte
; ML64:    encoding: [0x48,0xb8,A,A,A,A,A,A,A,A]
; ML64-NEXT: value: qvar, kind: FK_Data_8
  mov r9, OFFSET qvar+8
; DEFAULT: encoding: [0x49,0xc7,0xc1,A,A,A,A]
; ML64:    encoding: [0x49,0xb9,A,A,A,A,A,A,A,A]
  mov eax, OFFSET qvar
; CHECK:   encoding: [0xb8,A,A,A,A]
  mov rax, 1
; CHECK:   encoding: [0x48,0xc7,0xc0,0x01,0x00,0x00,0x00]

; [constant] is an absolute address, [label] is RIP-relative.
  mov eax, dword ptr gs:[30h]
; DEFAULT: encoding: [0x65,0x8b,0x05,0x30,0x00,0x00,0x00]
; ML64:    encoding: [0x65,0x8b,0x04,0x25,0x30,0x00,0x00,0x00]
  mov rax, qword ptr [1234h]
; DEFAULT: encoding: [0x48,0x8b,0x05,0x34,0x12,0x00,0x00]
; ML64:    encoding: [0x48,0x8b,0x04,0x25,0x34,0x12,0x00,0x00]
  lea rax, g
; DEFAULT: encoding: [0x48,0x8d,0x04,0x25,A,A,A,A]
; ML64:    encoding: [0x48,0x8d,0x05,A,A,A,A]
  mov eax, dvar
; CHECK:   encoding: [0x8b,0x05,A,A,A,A]

; Legacy prefixes: REP; then LOCK/mandatory, 67, 66, segment - or, before a
; REX/VEX, 67, 66, segment, LOCK/mandatory.
  lock add word ptr gs:[rcx], 1
; DEFAULT: encoding: [0x65,0x66,0xf0,0x83,0x01,0x01]
; ML64:    encoding: [0xf0,0x66,0x65,0x83,0x01,0x01]
  lock cmpxchg qword ptr gs:[rcx], rdx
; CHECK:   encoding: [0x65,0xf0,0x48,0x0f,0xb1,0x11]
  movdqa xmm0, xmmword ptr gs:[ecx]
; DEFAULT: encoding: [0x65,0x67,0x66,0x0f,0x6f,0x01]
; ML64:    encoding: [0x66,0x67,0x65,0x0f,0x6f,0x01]
  popcnt ax, word ptr gs:[rcx]
; DEFAULT: encoding: [0x65,0x66,0xf3,0x0f,0xb8,0x01]
; ML64:    encoding: [0xf3,0x66,0x65,0x0f,0xb8,0x01]
  popcnt rax, qword ptr gs:[rcx]
; CHECK:   encoding: [0x65,0xf3,0x48,0x0f,0xb8,0x01]
  mov ax, word ptr fs:[eax+ecx*2]
; DEFAULT: encoding: [0x64,0x67,0x66,0x8b,0x04,0x48]
; ML64:    encoding: [0x67,0x66,0x64,0x8b,0x04,0x48]
  rep movs word ptr [rdi], word ptr gs:[rsi]
; DEFAULT: encoding: [0xf3,0x65,0x66,0xa5]
; ML64:    encoding: [0xf3,0x66,0x65,0xa5]

; RIP-relative operand + immediate: IMAGE_REL_AMD64_REL32_<n> (object test:
; ml64-encoding-obj.asm).
  cmp bvar, 80h
; CHECK:   encoding: [0x80,0x3d,A,A,A,A,0x80]
; DEFAULT-NEXT: value: bvar-1, kind: reloc_riprel_4byte{{$}}
; ML64-NEXT: value: bvar-1, kind: reloc_riprel_4byte_masm_imm1
  cmp word ptr [wvar], 700h
; ML64:    kind: reloc_riprel_4byte_masm_imm2
  mov dword ptr [dvar+4], 1
; ML64:    kind: reloc_riprel_4byte_masm_imm4
f ENDP
g PROC
  ret
g ENDP
_TEXT ENDS
END
