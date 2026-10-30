//===--- CLWarnings.h - Maps some cl.exe warning ids  -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Diagnostic-related interfaces.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/CLWarnings.h"
#include "clang/Basic/DiagnosticCategories.h"
#include <optional>

using namespace clang;

std::optional<diag::Group>
clang::diagGroupFromCLWarningID(unsigned CLWarningID) {
  switch (CLWarningID) {
  // Each id was checked on both sides: cl.exe /W4 prints the Cxxxx for a small
  // trigger and clang-cl /Wall prints the group tag for the same trigger.
  // Ids with no matching clang tag stay unmapped (silent under /wd, cl style).
  case 4005: return diag::Group::MacroRedefined;
  case 4013: return diag::Group::ImplicitFunctionDeclare;
  case 4018: return diag::Group::SignCompare;
  case 4047: return diag::Group::IncompatiblePointerTypes;
  case 4068: return diag::Group::UnknownPragmas;
  case 4090: return diag::Group::IncompatiblePointerTypesDiscardsQualifiers;
  case 4100: return diag::Group::UnusedParameter;
  case 4101: return diag::Group::UnusedVariable;
  case 4133: return diag::Group::IncompatiblePointerTypes;
  case 4141: return diag::Group::DuplicateDeclSpecifier;
  case 4189: return diag::Group::UnusedVariable;
  case 4244: return diag::Group::Shorten64To32;
  case 4245: return diag::Group::SignConversion;
  case 4311: return diag::Group::PointerToIntCast;
  case 4389: return diag::Group::SignCompare;
  case 4456: return diag::Group::Shadow;
  case 4459: return diag::Group::Shadow;
  case 4552: return diag::Group::UnusedValue;
  case 4700: return diag::Group::Uninitialized;
  case 4702: return diag::Group::UnreachableCodeReturn;
  case 4706: return diag::Group::Parentheses;
  case 4715: return diag::Group::ReturnType;
  case 4724: return diag::Group::DivZero;
  case 4910: return diag::Group::DllexportExplicitInstantiationDecl;
  case 4996: return diag::Group::DeprecatedDeclarations;
  }
  return {};
}
