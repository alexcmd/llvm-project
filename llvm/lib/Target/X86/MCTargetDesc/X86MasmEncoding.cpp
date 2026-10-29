//===-- X86MasmEncoding.cpp - ML64.EXE-identical x86 encoding -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ML64.EXE 14.51 encoding rules (MCTargetOptions::X86MasmEncoding), each
// measured on ML64.EXE output bytes (left: ML64, right: LLVM default):
//  - reg,reg ADD/OR/ADC/SBB/AND/SUB/XOR/CMP/MOV: the "reg, r/m" opcode
//    (sub rdx,rcx = 48 2b d1 | 48 29 ca; mov r9,r8 = 4d 8b c8 | 4d 89 c1).
//  - TEST r,r: first operand in ModRM.reg (test rax,rcx = 48 85 c1 | 48 85 c8).
//  - xchg rax,rax = 48 90 | 90; xchg eax,eax = 90 | 87 c0.
//  - ret 0 = c3 | c2 00 00; push/pop fs/gs = 0f a0 | 66 0f a0 (64-bit forms).
//  - mov r64, OFFSET sym = movabs (48 b8 + ADDR64) | 48 c7 c0 + ADDR32.
//  - [constant] (also seg:[constant]) is absolute (8b 04 25 disp32), never
//    RIP-relative; [symbol] without base/index is RIP-relative (lea rax, label
//    = 48 8d 05 + REL32 | 48 8d 04 25 + ADDR32); [symbol+index*s] is absolute
//    (jmp qword ptr [tbl+rax*8] = ff 24 c5 + ADDR32 | RIP-relative without
//    the index, or an assertion).
//  - no VEX2 commutation: vpaddd xmm0,xmm1,xmm9 = c4 c1 71 fe c1 | c5 b1 fe c1,
//    vmovaps xmm0,xmm15 = c4 c1 78 28 c7 | c5 78 29 f8.
//  - legacy prefix order (reorderPrefixesForMasm), ALIGN nops (writeMasmNops).
// Elsewhere under the same option: RIP-relative operands followed by an
// immediate use IMAGE_REL_AMD64_REL32_<imm size> (X86MCCodeEmitter,
// X86WinCOFFObjectWriter), every RIP-relative operand keeps its relocation
// (X86AsmBackend::applyFixup), branches to a function in the same section are
// resolved (WinCOFFObjectWriter), '$' is the location counter (MasmParser).
// Not reproduced: ML64 picks movabs for 'mov r64, 0FFFFFFFFFFFFFFFFh' but
// 48 c7 c0 for 'mov r64, -1' (MASM's constants keep a sign; MCExpr does not).
//
//===----------------------------------------------------------------------===//

#include "X86MasmEncoding.h"
#include "X86BaseInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace llvm;

// sym, sym+c, sym-c, c+sym with no relocation specifier (no IMAGEREL etc.).
static bool isPlainSymbolic(const MCExpr *E) {
  if (const auto *S = dyn_cast<MCSymbolRefExpr>(E))
    return !S->getSpecifier();
  if (const auto *B = dyn_cast<MCBinaryExpr>(E)) {
    if (B->getOpcode() == MCBinaryExpr::Add &&
        isa<MCConstantExpr>(B->getLHS()))
      return isPlainSymbolic(B->getRHS());
    return (B->getOpcode() == MCBinaryExpr::Add ||
            B->getOpcode() == MCBinaryExpr::Sub) &&
           isa<MCConstantExpr>(B->getRHS()) && isPlainSymbolic(B->getLHS());
  }
  return false;
}

bool X86::rewriteForMasmEncoding(MCInst &MI, const MCInstrInfo &MCII,
                                 bool Is64Bit, StringRef Mnemonic) {
  bool Changed = false;
  // Memory operand: [constant] and [symbol+index*s] absolute, [symbol]
  // RIP-relative (x64).
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  int MemOp = X86II::getMemoryOperandNo(Desc.TSFlags);
  if (Is64Bit && MemOp >= 0) {
    MemOp += X86II::getOperandBias(Desc);
    MCOperand &Base = MI.getOperand(MemOp + X86::AddrBaseReg);
    const MCOperand &Index = MI.getOperand(MemOp + X86::AddrIndexReg);
    const MCOperand &Disp = MI.getOperand(MemOp + X86::AddrDisp);
    if (Base.getReg() == X86::RIP && (Disp.isImm() || Index.getReg())) {
      Base.setReg(MCRegister());
      Changed = true;
    } else if (!Base.getReg() && !Index.getReg() && Disp.isExpr() &&
               isPlainSymbolic(Disp.getExpr())) {
      Base.setReg(X86::RIP);
      Changed = true;
    }
  }

  unsigned NewOpc = 0;
  switch (MI.getOpcode()) {
#define TO_REV(OPC)                                                            \
  case X86::OPC##8rr:                                                          \
    NewOpc = X86::OPC##8rr_REV;                                                \
    break;                                                                     \
  case X86::OPC##16rr:                                                         \
    NewOpc = X86::OPC##16rr_REV;                                               \
    break;                                                                     \
  case X86::OPC##32rr:                                                         \
    NewOpc = X86::OPC##32rr_REV;                                               \
    break;                                                                     \
  case X86::OPC##64rr:                                                         \
    NewOpc = X86::OPC##64rr_REV;                                               \
    break;
    TO_REV(ADD)
    TO_REV(ADC)
    TO_REV(AND)
    TO_REV(CMP)
    TO_REV(OR)
    TO_REV(SBB)
    TO_REV(SUB)
    TO_REV(XOR)
    TO_REV(MOV)
#undef TO_REV
  case X86::TEST8rr:
  case X86::TEST16rr:
  case X86::TEST32rr:
  case X86::TEST64rr: {
    MCOperand Op0 = MI.getOperand(0);
    MI.getOperand(0) = MI.getOperand(1);
    MI.getOperand(1) = Op0;
    return true;
  }
  case X86::NOOP: // the 'xchg rax, rax' alias
    if (!Is64Bit || !Mnemonic.equals_insensitive("xchg"))
      return Changed;
    MI.setOpcode(X86::XCHG64ar);
    MI.addOperand(MCOperand::createReg(X86::RAX));
    MI.addOperand(MCOperand::createReg(X86::RAX));
    return true;
  case X86::XCHG32rr:
    if (!Is64Bit || MI.getOperand(2).getReg() != X86::EAX ||
        MI.getOperand(3).getReg() != X86::EAX)
      return Changed;
    MI.clear();
    MI.setOpcode(X86::NOOP);
    return true;
  case X86::RETI64:
    if (!MI.getOperand(0).isImm() || MI.getOperand(0).getImm() != 0)
      return Changed;
    MI.clear();
    MI.setOpcode(X86::RET64);
    return true;
  case X86::PUSHFS16:
    NewOpc = Is64Bit ? X86::PUSHFS64 : 0;
    break;
  case X86::PUSHGS16:
    NewOpc = Is64Bit ? X86::PUSHGS64 : 0;
    break;
  case X86::POPFS16:
    NewOpc = Is64Bit ? X86::POPFS64 : 0;
    break;
  case X86::POPGS16:
    NewOpc = Is64Bit ? X86::POPGS64 : 0;
    break;
  case X86::MOV64ri32:
    if (MI.getOperand(1).isExpr() && isPlainSymbolic(MI.getOperand(1).getExpr()))
      NewOpc = X86::MOV64ri;
    break;
  default:
    break;
  }
  if (!NewOpc)
    return Changed;
  MI.setOpcode(NewOpc);
  return true;
}

void X86::reorderPrefixesForMasm(SmallVectorImpl<char> &CB, uint64_t Begin,
                                 uint64_t End, uint64_t TSFlags,
                                 unsigned InstFlags, bool HasRexOrVex) {
  // ML64: explicit REP/REPNE first; then without REX/VEX
  //   LOCK|mandatory 66/F2/F3, 67, 66 (operand size), segment
  // and with REX/VEX
  //   67, 66 (operand size), segment, LOCK|mandatory.
  // e.g. lock add word ptr gs:[rcx],1 = f0 66 65 | 65 66 f0;
  //      lock cmpxchg qword ptr gs:[rcx],rdx = 65 f0 48 | 65 f0 48;
  //      movdqa xmm0, gs:[ecx] = 66 67 65 | 65 67 66;
  //      popcnt ax, word ptr gs:[rcx] = f3 66 65 | 65 66 f3.
  enum { Rep, Group1, AdSize, OpSize, Seg };
  uint64_t Mandatory = TSFlags & X86II::OpPrefixMask;
  bool ExplicitRep = (TSFlags & X86II::REP) || (InstFlags & X86::IP_HAS_REPEAT);
  bool ExplicitRepNE = InstFlags & X86::IP_HAS_REPEAT_NE;
  bool NoTrack = (TSFlags & X86II::NOTRACK) || (InstFlags & X86::IP_HAS_NOTRACK);
  auto Rank = [&](uint8_t B) -> unsigned {
    unsigned C;
    switch (B) {
    case 0xF3:
      C = ExplicitRep && Mandatory != X86II::XS ? Rep : Group1;
      break;
    case 0xF2:
      C = ExplicitRepNE && Mandatory != X86II::XD ? Rep : Group1;
      break;
    case 0xF0:
      C = Group1;
      break;
    case 0x67:
      C = AdSize;
      break;
    case 0x66:
      C = Mandatory == X86II::PD ? Group1 : OpSize;
      break;
    case 0x3E:
      C = NoTrack ? Group1 : Seg;
      break;
    default: // 26 2e 36 64 65
      C = Seg;
      break;
    }
    if (!HasRexOrVex)
      return C;
    return C == Group1 ? 5 : C;
  };
  auto IsLegacy = [](uint8_t B) {
    switch (B) {
    case 0x26: case 0x2E: case 0x36: case 0x3E: case 0x64: case 0x65:
    case 0x66: case 0x67: case 0xF0: case 0xF2: case 0xF3:
      return true;
    }
    return false;
  };
  uint64_t N = Begin;
  while (N < End && IsLegacy(uint8_t(CB[N])))
    ++N;
  std::stable_sort(CB.begin() + Begin, CB.begin() + N, [&](char A, char B) {
    return Rank(uint8_t(A)) < Rank(uint8_t(B));
  });
}

void X86::writeMasmNops(raw_ostream &OS, uint64_t Count) {
  // 15-byte pieces, then the rest. 1..9 bytes: the usual multi-byte nops;
  // 10..15: (n-8) x 66 + 0f 1f 84 00 00 00 00 00 (LLVM: 66 .. 2e 0f 1f 84 ..).
  static const char Nops[9][10] = {
      "\x90",
      "\x66\x90",
      "\x0f\x1f\x00",
      "\x0f\x1f\x40\x00",
      "\x0f\x1f\x44\x00\x00",
      "\x66\x0f\x1f\x44\x00\x00",
      "\x0f\x1f\x80\x00\x00\x00\x00",
      "\x0f\x1f\x84\x00\x00\x00\x00\x00",
      "\x66\x0f\x1f\x84\x00\x00\x00\x00\x00",
  };
  while (Count) {
    uint64_t Len = std::min<uint64_t>(Count, 15);
    if (Len <= 9) {
      OS.write(Nops[Len - 1], Len);
    } else {
      for (uint64_t I = 0; I != Len - 8; ++I)
        OS << '\x66';
      OS.write(Nops[7], 8);
    }
    Count -= Len;
  }
}
