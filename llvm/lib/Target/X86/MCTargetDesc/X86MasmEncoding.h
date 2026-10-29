//===-- X86MasmEncoding.h - ML64.EXE-identical x86 encoding ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Opt-in encoding choices of Microsoft ML64.EXE (MCTargetOptions::
// X86MasmEncoding; llvm-ml /ml64-encoding, llvm-mc -x86-masm-encoding). Every
// rule was measured against ML64.EXE 14.51 output bytes; the default LLVM
// encodings are left untouched when the option is off.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86MASMENCODING_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86MASMENCODING_H

#include "llvm/ADT/StringRef.h"
#include <cstdint>

namespace llvm {
class MCInst;
class MCInstrInfo;
class raw_ostream;
template <typename T> class SmallVectorImpl;

namespace X86 {

/// Rewrite a matched instruction to the form ML64.EXE selects (reg,reg ALU and
/// MOV in the MRMSrcReg form, TEST operand order, xchg/ret/push-seg/movabs
/// choices, absolute vs RIP-relative memory operands). \p Mnemonic is the
/// parsed mnemonic. Applied once, right before the instruction is emitted.
bool rewriteForMasmEncoding(MCInst &MI, const MCInstrInfo &MCII, bool Is64Bit,
                            StringRef Mnemonic);

/// Put the legacy prefixes of the instruction encoded at CB[Begin, End) into
/// ML64.EXE order. \p HasRexOrVex: a REX/VEX/EVEX/XOP prefix follows them.
void reorderPrefixesForMasm(SmallVectorImpl<char> &CB, uint64_t Begin,
                            uint64_t End, uint64_t TSFlags, unsigned InstFlags,
                            bool HasRexOrVex);

/// ALIGN padding as ML64.EXE writes it.
void writeMasmNops(raw_ostream &OS, uint64_t Count);

} // namespace X86
} // namespace llvm

#endif
