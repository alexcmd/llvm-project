//===--- CGMSOutlinedBuiltin.cpp - x86 builtins outside their ISA (MS) ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// UE-SSE42-MACRO-INTRIN: cl lets any function use any x86 intrinsic, whatever
// /arch says. clang's macro-implemented intrinsics (an immediate operand, e.g.
// _mm_cmpistri) call the target builtin right in the caller; in C++ the
// headers wrap them in always_inline templates, which the MS target-feature
// relief and the MS-INTRIN-ABI-THUNK thunks handle, but C has no templates
// and the call was "needs target feature". Here such a builtin call, written
// in a system header macro, is emitted into an internal always_inline
// function with the caller's features plus the builtin's; the thunk pass
// (CodeGenModule::emitMSIntrinsicFeatureThunks) then calls it out of line, so
// only the intrinsic's own instructions need the ISA, as with cl.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Function.h"
#include <map>

using namespace clang;
using namespace CodeGen;

std::optional<RValue>
CodeGenFunction::EmitMSOutlinedTargetBuiltin(GlobalDecl GD, unsigned BuiltinID,
                                             const CallExpr *E) {
  ASTContext &Ctx = getContext();
  if (!getLangOpts().isCompatibleWithMSVC() || !getTarget().getTriple().isX86())
    return std::nullopt;
  const auto *CallerFD = dyn_cast_or_null<FunctionDecl>(CurCodeDecl);
  if (!CallerFD || !CurFn)
    return std::nullopt;
  StringRef FeatureList = Ctx.BuiltinInfo.getRequiredFeatures(BuiltinID);
  if (FeatureList.empty())
    return std::nullopt;
  llvm::StringMap<bool> CallerMap;
  Ctx.getFunctionFeatureMap(CallerMap, CallerFD);
  if (Builtin::evaluateRequiredTargetFeatures(FeatureList, CallerMap))
    return std::nullopt;
  // Only the intrinsics of the system headers (a macro around the builtin).
  SourceManager &SM = Ctx.getSourceManager();
  if (!E->getBeginLoc().isMacroID() ||
      !SM.isInSystemHeader(SM.getSpellingLoc(E->getBeginLoc())))
    return std::nullopt;
  QualType RetTy = E->getType();
  if (!RetTy->isVoidType() && !RetTy->isScalarType() && !RetTy->isVectorType())
    return std::nullopt;

  unsigned ICEArgs = 0;
  ASTContext::GetBuiltinTypeError Err;
  Ctx.GetBuiltinType(BuiltinID, Err, &ICEArgs);
  if (Err != ASTContext::GE_None)
    return std::nullopt;

  // Arguments: the immediates stay in the call, the others become
  // parameters, evaluated here.
  FunctionArgList Params;
  CallArgList Args;
  SmallVector<Expr *, 4> NewArgs;
  for (unsigned I = 0, N = E->getNumArgs(); I != N; ++I) {
    const Expr *A = E->getArg(I);
    if (I < 32 && (ICEArgs & (1u << I))) {
      NewArgs.push_back(const_cast<Expr *>(A));
      continue;
    }
    QualType T = A->getType();
    if (!T->isScalarType() && !T->isVectorType())
      return std::nullopt;
    auto *P = ImplicitParamDecl::Create(Ctx, Ctx.getTranslationUnitDecl(),
                                        A->getExprLoc(), /*Id=*/nullptr, T,
                                        ImplicitParamKind::Other);
    P->setIsUsed();
    Params.push_back(P);
    auto *Ref = DeclRefExpr::Create(Ctx, NestedNameSpecifierLoc(),
                                    SourceLocation(), P, false,
                                    A->getExprLoc(), T, VK_LValue);
    NewArgs.push_back(ImplicitCastExpr::Create(Ctx, T, CK_LValueToRValue, Ref,
                                               nullptr, VK_PRValue,
                                               FPOptionsOverride()));
  }
  for (unsigned I = 0, N = E->getNumArgs(), K = 0; I != N; ++I)
    if (!(I < 32 && (ICEArgs & (1u << I))))
      Args.add(RValue::get(EmitScalarExpr(E->getArg(I))),
               cast<ImplicitParamDecl>(Params[K++])->getType());
  CallExpr *NewE = CallExpr::Create(
      Ctx, const_cast<Expr *>(E->getCallee()), NewArgs, RetTy,
      E->getValueKind(), E->getRParenLoc(), FPOptionsOverride());

  const CGFunctionInfo &FI =
      CGM.getTypes().arrangeBuiltinFunctionDeclaration(RetTy, Params);
  llvm::FunctionType *FTy = CGM.getTypes().GetFunctionType(FI);
  std::string Name =
      "__openrt_ms_builtin." + Ctx.BuiltinInfo.getName(BuiltinID);
  auto *Fn = llvm::Function::Create(FTy, llvm::GlobalValue::InternalLinkage,
                                    Name, &CGM.getModule());
  CGM.SetInternalFunctionAttributes(GlobalDecl(), Fn, FI);
  {
    CodeGenFunction CGF(CGM);
    CGF.StartFunction(GlobalDecl(), RetTy, Fn, FI, Params);
    RValue R = CGF.EmitBuiltinExpr(GD, BuiltinID, NewE, ReturnValueSlot());
    if (!RetTy->isVoidType())
      CGF.Builder.CreateStore(R.getScalarVal(), CGF.ReturnValue);
    CGF.FinishFunction();
  }

  // Features: the caller's plus the builtin's (with what they imply).
  llvm::StringMap<bool> Map = CallerMap;
  SmallVector<StringRef, 4> Req;
  std::string Flat = FeatureList.str();
  for (char &C : Flat)
    if (C == '|' || C == '(' || C == ')')
      C = ',';
  StringRef(Flat).split(Req, ',', -1, /*KeepEmpty=*/false);
  for (StringRef F : Req)
    getTarget().setFeatureEnabled(Map, F, true);
  std::map<std::string, bool> Sorted;
  for (const auto &KV : Map)
    Sorted[KV.getKey().str()] = KV.getValue();
  std::string Features;
  for (const auto &KV : Sorted) {
    if (!Features.empty())
      Features += ',';
    Features += (KV.second ? "+" : "-") + KV.first;
  }
  Fn->addFnAttr("target-features", Features);
  Fn->removeFnAttr(llvm::Attribute::NoInline);
  Fn->removeFnAttr(llvm::Attribute::OptimizeNone);
  Fn->addFnAttr(llvm::Attribute::AlwaysInline);

  return EmitCall(FI, CGCallee::forDirect(Fn), ReturnValueSlot(), Args);
}
