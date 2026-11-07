//===--- CLWarnings.h - Maps some cl.exe warning ids  -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_CLWARNINGS_H
#define LLVM_CLANG_BASIC_CLWARNINGS_H

#include "llvm/ADT/SmallVector.h"
#include <optional>

namespace clang {

namespace diag {
enum class Group;
}

/// For cl.exe warning IDs that cleany map to clang diagnostic groups,
/// returns the corresponding group. Else, returns an empty Optional.
std::optional<diag::Group> diagGroupFromCLWarningID(unsigned);

/// Every group a cl.exe warning id stands for: the one above plus the groups
/// clang splits the same cl situation into (C5054: enum-enum arithmetic, named
/// or anonymous, and enum comparison). Empty when unmapped.
llvm::SmallVector<diag::Group, 4> diagGroupsFromCLWarningID(unsigned);

/// The cl.exe warning id of a diagnostic group (the lowest id mapping to
/// exactly this group), 0 if none.
unsigned clWarningIDFromDiagGroup(diag::Group);

/// CL-WARNING-DEFAULTS: a clang warning group that clang-cl turns on (by
/// default, -Wall or -Wextra) and the lowest cl.exe /W level that reports the
/// same situation. Level 0: cl.exe has no on-by-default counterpart, so the
/// group stays off at /W0../W4 (still available as -W<group>, /clang:-W<group>
/// or through a mapped /w<l><n>). Parent: a group listed earlier whose -Wno-
/// also switches this one off, so it is switched back on when kept. Also: a
/// related group name; the user naming it keeps the entry out, too.
struct CLWarningLevel {
  const char *Group;
  unsigned Level;
  const char *Parent;
  const char *Also;
  /// The group is off by default in clang: turn it on from this level.
  bool Enable = false;
};

/// The clang-only warnings clang-cl gates by /W level, parents first.
const CLWarningLevel *clWarningLevels(unsigned &Count);

} // end namespace clang

#endif // LLVM_CLANG_BASIC_CLWARNINGS_H
