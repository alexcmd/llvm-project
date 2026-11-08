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
#include "clang/Basic/DiagnosticIDs.h"
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
  // UE-C4244-SHORTEN: 64-to-32 narrowing; from size_t it is C4267.
  case 4244: return diag::Group::ShortenInt64To32;
  case 4267: return diag::Group::ShortenSizeT;
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
  // CL-WARNING-DEFAULTS: ids found by the /W4 survey (tasks/compat/13): cl.exe
  // and clang-cl report the same line of the same probe. Groups without a
  // named def are looked up by name.
  const char *Name = nullptr;
  switch (CLWarningID) {
  case 4010: Name = "comment"; break;
  case 4057: Name = "pointer-sign"; break;
  case 4062: Name = "switch"; break;
  case 4099: Name = "mismatched-tags"; break;
  case 4102: Name = "unused-label"; break;
  // UE-PRAGMA-PACK-C4103: #pragma pack changed across an #include.
  case 4103: Name = "pragma-pack"; break;
  case 4131: Name = "deprecated-non-prototype"; break;
  case 4150: Name = "delete-incomplete"; break;
  case 4191: Name = "cast-function-type-mismatch"; break;
  case 4201: Name = "microsoft-anon-tag"; break;
  case 4263: Name = "overloaded-virtual"; break;
  case 4264: Name = "overloaded-virtual"; break;
  case 4305: Name = "constant-conversion"; break;
  case 4307: Name = "integer-overflow"; break;
  case 4309: Name = "constant-conversion"; break;
  case 4390: Name = "empty-body"; break;
  case 4431: Name = "implicit-int"; break;
  case 4463: Name = "bitfield-constant-conversion"; break;
  case 4471: Name = "microsoft-enum-forward-reference"; break;
  case 4505: Name = "unused-function"; break;
  // UE-NOISE: expression before comma has no effect (off by default in cl).
  case 4548: Name = "unused-comma-left-operand"; break;
  case 4554: Name = "shift-op-parentheses"; break;
  case 4595: Name = "inline-new-delete"; break;
  case 4596: Name = "microsoft-extra-qualification"; break;
  case 4625: Name = "defaulted-function-deleted"; break;
  case 4701: Name = "sometimes-uninitialized"; break;
  case 4723: Name = "division-by-zero"; break;
  // UE-NARROWING-C4838: narrowing in a variable's aggregate initializer.
  case 4838: Name = "microsoft-init-list-narrowing"; break;
  case 5030: Name = "unknown-attributes"; break;
  case 5038: Name = "reorder-ctor"; break;
  case 5054: Name = "deprecated-enum-enum-conversion"; break;
  case 5061: Name = "deprecated-comma-subscript"; break;
  case 5205: Name = "delete-abstract-non-virtual-dtor"; break;
  case 5263: Name = "pessimizing-move"; break;
  case 5266: Name = "ignored-qualifiers"; break;
  }
  if (Name)
    return DiagnosticIDs::getGroupForWarningOption(Name);
  return {};
}

llvm::SmallVector<diag::Group, 4>
clang::diagGroupsFromCLWarningID(unsigned CLWarningID) {
  llvm::SmallVector<diag::Group, 4> Groups;
  if (auto G = diagGroupFromCLWarningID(CLWarningID))
    Groups.push_back(*G);
  // UE-WD5054: cl's C5054 is one id for what clang reports under three groups
  // (named and anonymous enum arithmetic, enum comparison); measured with
  // cl /std:c++20 /W4.
  const char *Extra[3] = {};
  if (CLWarningID == 5054) {
    Extra[0] = "deprecated-anon-enum-enum-conversion";
    Extra[1] = "deprecated-enum-compare";
  }
  for (const char *Name : Extra)
    if (Name)
      if (auto G = DiagnosticIDs::getGroupForWarningOption(Name))
        Groups.push_back(*G);
  return Groups;
}

unsigned clang::clWarningIDFromDiagGroup(diag::Group Group) {
  for (unsigned ID = 4000; ID != 6000; ++ID)
    for (diag::Group G : diagGroupsFromCLWarningID(ID))
      if (G == Group)
        return ID;
  return 0;
}

// CL-WARNING-DEFAULTS: decided from the /W1../W4 and /Wall survey of probe TUs
// and the uelike corpus against cl.exe (table in tasks/compat/13). Entries
// with level 0 are reported by clang only (or by cl.exe only when enabled by
// hand, see the id map above); a non-zero level is the cl.exe level of the
// matching warning. Parents come before their children.
static const CLWarningLevel CLLevels[] = {
    // Parents that are switched off while some children stay.
    {"parentheses", 4, nullptr, nullptr},
    {"enum-compare", 0, nullptr, nullptr},
    // Off: clang-only.
    {"array-bounds", 0, nullptr, nullptr},
    {"array-compare", 0, nullptr, nullptr},
    {"bitwise-instead-of-logical", 0, nullptr, nullptr},
    {"char-subscripts", 0, nullptr, nullptr},
    {"constant-logical-operand", 0, nullptr, nullptr},
    {"dangling-gsl", 0, nullptr, nullptr},
    {"delete-non-abstract-non-virtual-dtor", 0, nullptr, nullptr},
    {"deprecated-copy", 0, nullptr, nullptr},
    {"deprecated-literal-operator", 0, nullptr, nullptr},
    {"deprecated-this-capture", 0, nullptr, nullptr},
    {"deprecated-volatile", 0, nullptr, nullptr},
    {"dynamic-class-memaccess", 0, nullptr, nullptr},
    {"extern-initializer", 0, nullptr, nullptr},
    {"extra-tokens", 0, nullptr, nullptr},
    {"final-dtor-non-final-class", 0, nullptr, nullptr},
    {"inconsistent-missing-override", 0, nullptr, nullptr},
    {"int-in-bool-context", 0, nullptr, nullptr},
    {"invalid-offsetof", 0, nullptr, nullptr},
    {"microsoft-abstract", 0, nullptr, nullptr},
    {"microsoft-goto", 0, nullptr, nullptr},
    {"microsoft-include", 0, nullptr, nullptr},
    {"microsoft-pure-definition", 0, nullptr, nullptr},
    {"microsoft-sealed", 0, nullptr, nullptr},
    {"microsoft-string-literal-from-predefined", 0, nullptr, nullptr},
    {"microsoft-template", 0, nullptr, nullptr},
    {"microsoft-void-pseudo-dtor", 0, nullptr, nullptr},
    {"missing-braces", 0, nullptr, nullptr},
    {"missing-field-initializers", 0, nullptr, nullptr},
    {"nonportable-include-path", 0, nullptr, nullptr},
    {"nontrivial-memcall", 0, nullptr, nullptr},
    {"null-dereference", 0, nullptr, nullptr},
    {"null-pointer-arithmetic", 0, nullptr, nullptr},
    {"potentially-evaluated-expression", 0, nullptr, nullptr},
    {"pragma-once-outside-header", 0, nullptr, nullptr},
    {"range-loop-construct", 0, nullptr, nullptr},
    {"redundant-move", 0, nullptr, nullptr},
    {"self-assign", 0, nullptr, nullptr},
    {"self-move", 0, nullptr, nullptr},
    {"sizeof-array-argument", 0, nullptr, nullptr},
    {"sizeof-pointer-memaccess", 0, nullptr, nullptr},
    {"string-plus-int", 0, nullptr, nullptr},
    {"tautological-compare", 0, nullptr, nullptr},
    {"undefined-var-template", 0, nullptr, nullptr},
    {"unevaluated-expression", 0, nullptr, nullptr},
    {"unnecessary-virtual-specifier", 0, nullptr, nullptr},
    {"unqualified-std-cast-call", 0, nullptr, nullptr},
    {"unused-but-set-parameter", 0, nullptr, nullptr},
    {"unused-but-set-variable", 0, nullptr, nullptr},
    {"unused-lambda-capture", 0, nullptr, nullptr},
    {"unused-local-typedef", 0, nullptr, nullptr},
    {"unused-private-field", 0, nullptr, nullptr},
    // /Zc:strictStrings (also from /permissive-) names the child group.
    {"writable-strings", 0, nullptr,
     "c++11-compat-deprecated-writable-strings"},
    // UE-NOISE: cl 14.51 is silent on these (measured on minimal repros; the
    // UnrealEditor build: cl 0 warnings, openrt 18256 + 1772 + 1743 + 686
    // + 26 + 6 of them).
    {"implicit-exception-spec-mismatch", 0, nullptr, nullptr},
    {"extra-qualification", 0, nullptr, nullptr},
    {"unused-comma-left-operand", 0, nullptr, nullptr},
    {"microsoft-unqualified-friend", 0, nullptr, nullptr},
    {"microsoft-cast", 0, nullptr, nullptr},
    {"invalid-unevaluated-string", 0, nullptr, nullptr},
    // UE-WD5054: cl has no counterpart of the ?: enum mismatch.
    {"deprecated-enum-compare-conditional", 0, nullptr, nullptr},
    // Off: the cl.exe counterpart is off by default (enable via /w<l><n>).
    {"cast-function-type-mismatch", 0, nullptr, nullptr},
    {"comment", 0, nullptr, nullptr},
    {"defaulted-function-deleted", 0, nullptr, nullptr},
    {"ignored-qualifiers", 0, nullptr, nullptr},
    {"microsoft-enum-forward-reference", 0, nullptr, nullptr},
    {"microsoft-extra-qualification", 0, nullptr, nullptr},
    {"overloaded-virtual", 0, nullptr, nullptr},
    {"pessimizing-move", 0, nullptr, nullptr},
    {"reorder-ctor", 0, nullptr, nullptr},
    {"switch", 0, nullptr, nullptr},
    // Children of "parentheses".
    {"bitwise-conditional-parentheses", 0, nullptr, nullptr},
    {"bitwise-op-parentheses", 0, nullptr, nullptr},
    {"dangling-else", 0, nullptr, nullptr},
    {"logical-not-parentheses", 0, nullptr, nullptr},
    {"logical-op-parentheses", 0, nullptr, nullptr},
    {"overloaded-shift-op-parentheses", 0, nullptr, nullptr},
    {"parentheses-equality", 0, nullptr, nullptr},
    {"shift-op-parentheses", 3, "parentheses", nullptr},
    // Child of "enum-compare".
    {"deprecated-enum-compare", 4, "enum-compare", nullptr},
    // Kept from the cl.exe level of the matching warning.
    {"constant-conversion", 2, nullptr, nullptr},
    {"delete-incomplete", 2, nullptr, nullptr},
    {"implicit-const-int-float-conversion", 2, nullptr, nullptr},
    {"integer-overflow", 2, nullptr, nullptr},
    {"literal-conversion", 2, nullptr, nullptr},
    {"mismatched-tags", 2, nullptr, nullptr},
    {"deprecated-declarations", 3, nullptr, nullptr},
    {"division-by-zero", 3, nullptr, nullptr},
    {"empty-body", 3, nullptr, nullptr},
    {"implicit-function-declaration", 3, nullptr, nullptr},
    {"incompatible-pointer-types", 3, nullptr, nullptr},
    {"inline-new-delete", 3, nullptr, nullptr},
    {"sign-compare", 3, nullptr, nullptr},
    {"unknown-attributes", 3, nullptr, nullptr},
    {"unused-label", 3, nullptr, nullptr},
    {"unused-variable", 3, nullptr, nullptr},
    {"bitfield-constant-conversion", 4, nullptr, nullptr},
    {"compare-distinct-pointer-types", 4, nullptr, nullptr},
    {"delete-abstract-non-virtual-dtor", 4, nullptr, nullptr},
    {"deprecated-comma-subscript", 4, nullptr, nullptr},
    {"deprecated-enum-enum-conversion", 4, nullptr, nullptr},
    {"deprecated-anon-enum-enum-conversion", 4, nullptr, nullptr},
    {"deprecated-non-prototype", 4, nullptr, nullptr},
    {"implicit-int", 4, nullptr, nullptr},
    {"microsoft-anon-tag", 4, nullptr, nullptr},
    {"pointer-sign", 4, nullptr, nullptr},
    {"single-bit-bitfield-constant-conversion", 4, nullptr, nullptr},
    {"sometimes-uninitialized", 4, nullptr, nullptr},
    {"unused-function", 4, nullptr, nullptr},
    {"unused-parameter", 4, nullptr, nullptr},
    // UE-C4244-SHORTEN: off by default in clang, cl level 3 (C4244, C4267).
    {"shorten-int64-to-32", 3, nullptr, nullptr, true},
    {"shorten-size-t", 3, nullptr, nullptr, true},
};

const CLWarningLevel *clang::clWarningLevels(unsigned &Count) {
  Count = sizeof(CLLevels) / sizeof(CLLevels[0]);
  return CLLevels;
}
