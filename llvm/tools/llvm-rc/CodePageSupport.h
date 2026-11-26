//===-- CodePageSupport.h - Code pages of resource scripts ------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// openrt RC-STRING-CHARS: what rc.exe does with the code page of a script:
// the lead bytes of double-byte pages (a lead byte takes the next byte, even
// a quote) and writing a character in a code page with best fit.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_LLVMRC_CODEPAGESUPPORT_H
#define LLVM_TOOLS_LLVMRC_CODEPAGESUPPORT_H

#include "llvm/ADT/SmallVector.h"

namespace llvm {
namespace rc {

/// A double-byte code page (932, 936, 949, 950, 1361; on a Windows host also
/// any other page the system reads with two bytes).
bool isDoubleByteCodePage(int CodePage);

/// \p B starts a two-byte character in \p CodePage (observed:
/// IsDBCSLeadByteEx; the five pages above are built in).
bool isCodePageLeadByte(int CodePage, unsigned char B);

/// Appends code point \p C written in \p CodePage with best fit, as rc.exe
/// writes a narrow string ('?' where there is none). 1252 and 65001 are the
/// same on every host; other pages use the Windows API on a Windows host and
/// llvm's conversion without best fit elsewhere. False when the page cannot be
/// written on this host.
bool encodeBestFit(int CodePage, uint32_t C, SmallVectorImpl<char> &Out);

} // namespace rc
} // namespace llvm

#endif
