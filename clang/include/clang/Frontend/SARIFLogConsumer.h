//===--- SARIFLogConsumer.h - SARIF log file of the diagnostics -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// UE-EXPLOG-SARIF: clang-cl /experimental:log <path>. Next to the normal
// output, the warnings and errors of the translation unit are written as a
// SARIF 2.1.0 log in the shape cl 14.51 writes: one run, one result per
// warning or error (ruleId = cl warning id when known, "level": "error" for
// errors), the notes that follow as relatedLocations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_SARIFLOGCONSUMER_H
#define LLVM_CLANG_FRONTEND_SARIFLOGCONSUMER_H

#include "clang/Basic/Diagnostic.h"
#include "llvm/ADT/StringRef.h"
#include <memory>

namespace clang {

std::unique_ptr<DiagnosticConsumer> createSARIFLogConsumer(llvm::StringRef Path);

} // namespace clang

#endif
