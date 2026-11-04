//===- JMCInstrumenter.cpp - JMC Instrumentation --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// JMCInstrumenter pass:
// - instrument each function with a call to __CheckForDebuggerJustMyCode. The
//   sole argument should be defined in .msvcjmc. Each flag is 1 byte initilized
//   to 1.
// - create the dummy COMDAT function __JustMyCode_Default to prevent linking
//   error if __CheckForDebuggerJustMyCode is not available.
// - For MSVC:
//   add "/alternatename:__CheckForDebuggerJustMyCode=__JustMyCode_Default" to
//   "llvm.linker.options"
//   For ELF:
//   Rename __JustMyCode_Default to __CheckForDebuggerJustMyCode and mark it as
//   weak symbol.
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/JMCInstrumenter.h"
#include "llvm/ADT/SmallString.h"
#include <array>
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/DJB.h"
#include "llvm/Support/Path.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "jmc-instrumenter"

static bool runImpl(Module &M);
namespace {
struct JMCInstrumenter : public ModulePass {
  static char ID;
  JMCInstrumenter() : ModulePass(ID) {}
  bool runOnModule(Module &M) override { return runImpl(M); }
};
char JMCInstrumenter::ID = 0;
} // namespace

PreservedAnalyses JMCInstrumenterPass::run(Module &M, ModuleAnalysisManager &) {
  bool Changed = runImpl(M);
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

INITIALIZE_PASS(
    JMCInstrumenter, DEBUG_TYPE,
    "Instrument function entry with call to __CheckForDebuggerJustMyCode",
    false, false)

ModulePass *llvm::createJMCInstrumenterPass() { return new JMCInstrumenter(); }

namespace {
const char CheckFunctionName[] = "__CheckForDebuggerJustMyCode";

// CRC-32, reflected polynomial 0xEDB88320, initial value 0, no final xor (the
// register is not inverted, unlike zlib crc32). cl hashes this way (see
// tasks/compat/data/jmc_hash.md).
uint32_t msvcJMCHash(StringRef Path) {
  static const auto Table = [] {
    std::array<uint32_t, 256> T{};
    for (uint32_t I = 0; I != 256; ++I) {
      uint32_t C = I;
      for (int K = 0; K != 8; ++K)
        C = (C >> 1) ^ (C & 1 ? 0xEDB88320u : 0u);
      T[I] = C;
    }
    return T;
  }();
  uint32_t Crc = 0;
  for (unsigned char C : Path)
    Crc = Table[(Crc & 0xFF) ^ C] ^ (Crc >> 8);
  return Crc;
}

std::string getFlagName(const DIFile &File, bool UseX86FastCall, bool IsMSVC) {
  if (IsMSVC) {
    // cl: full path (directory + file name), '/' -> '\', dots removed, case
    // as spelled. The hash covers the whole path; the suffix is the file name
    // (fname + ext) with '.' replaced by '@'.
    SmallString<256> FullPath(File.getDirectory());
    sys::path::append(FullPath, sys::path::Style::windows_backslash,
                      File.getFilename());
    for (char &C : FullPath)
      if (C == '/')
        C = '\\';
    sys::path::remove_dots(FullPath, /*remove_dot_dot=*/true,
                           sys::path::Style::windows_backslash);
    std::string Suffix;
    for (auto C :
         sys::path::filename(FullPath, sys::path::Style::windows_backslash))
      Suffix.push_back(C == '.' ? '@' : C);
    return (UseX86FastCall ? "_" : "__") +
           utohexstr(msvcJMCHash(FullPath), /*LowerCase=*/false,
                     /*Width=*/8) +
           "_" + Suffix;
  }
  // ELF: unchanged.
  // absolute windows path:           windows_backslash
  // relative windows backslash path: windows_backslash
  // relative windows slash path:     posix
  // absolute posix path:             posix
  // relative posix path:             posix
  sys::path::Style PathStyle =
      has_root_name(File.getDirectory(), sys::path::Style::windows_backslash) ||
              File.getDirectory().contains("\\") ||
              File.getFilename().contains("\\")
          ? sys::path::Style::windows_backslash
          : sys::path::Style::posix;
  // Best effort path normalization. This is to guarantee an unique flag symbol
  // is produced for the same directory. Some builds may want to use relative
  // paths, or paths with a specific prefix (see the -fdebug-compilation-dir
  // flag), so only hash paths in debuginfo. Don't expand them to absolute
  // paths.
  SmallString<256> FilePath(File.getDirectory());
  sys::path::append(FilePath, PathStyle, File.getFilename());
  sys::path::native(FilePath, PathStyle);
  sys::path::remove_dots(FilePath, /*remove_dot_dot=*/true, PathStyle);

  // The naming convention for the flag name is __<hash>_<file name> with '.' in
  // <file name> replaced with '@'. For example C:\file.any.c would have a flag
  // __D032E919_file@any@c. The naming convention match MSVC's format however
  // the match is not required to make JMC work. The hashing function used here
  // is different from MSVC's.

  std::string Suffix;
  for (auto C : sys::path::filename(FilePath, PathStyle))
    Suffix.push_back(C == '.' ? '@' : C);

  sys::path::remove_filename(FilePath, PathStyle);
  return (UseX86FastCall ? "_" : "__") +
         utohexstr(djbHash(FilePath), /*LowerCase=*/false,
                   /*Width=*/8) +
         "_" + Suffix;
}

void attachDebugInfo(GlobalVariable &GV, DICompileUnit *CU, DIFile *File) {
  if (!CU)
    return;
  Module &M = *GV.getParent();
  DIBuilder DB(M, false, CU);

  auto *DType =
      DB.createBasicType("unsigned char", 8, dwarf::DW_ATE_unsigned_char,
                         llvm::DINode::FlagArtificial);

  auto *DGVE = DB.createGlobalVariableExpression(
      CU, GV.getName(), /*LinkageName=*/StringRef(), File,
      /*LineNo=*/0, DType, /*IsLocalToUnit=*/true, /*IsDefined=*/true);
  GV.addMetadata(LLVMContext::MD_dbg, *DGVE);
  DB.finalize();
}

FunctionType *getCheckFunctionType(LLVMContext &Ctx) {
  Type *VoidTy = Type::getVoidTy(Ctx);
  PointerType *VoidPtrTy = PointerType::getUnqual(Ctx);
  return FunctionType::get(VoidTy, VoidPtrTy, false);
}

Function *createDefaultCheckFunction(Module &M, bool UseX86FastCall) {
  LLVMContext &Ctx = M.getContext();
  const char *DefaultCheckFunctionName =
      UseX86FastCall ? "_JustMyCode_Default" : "__JustMyCode_Default";
  // Create the function.
  Function *DefaultCheckFunc =
      Function::Create(getCheckFunctionType(Ctx), GlobalValue::ExternalLinkage,
                       DefaultCheckFunctionName, &M);
  DefaultCheckFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
  DefaultCheckFunc->addParamAttr(0, Attribute::NoUndef);
  if (UseX86FastCall)
    DefaultCheckFunc->addParamAttr(0, Attribute::InReg);

  BasicBlock *EntryBB = BasicBlock::Create(Ctx, "", DefaultCheckFunc);
  ReturnInst::Create(Ctx, EntryBB);
  return DefaultCheckFunc;
}
} // namespace

bool runImpl(Module &M) {
  bool Changed = false;
  LLVMContext &Ctx = M.getContext();
  Triple ModuleTriple(M.getTargetTriple());
  bool IsMSVC = ModuleTriple.isKnownWindowsMSVCEnvironment();
  bool IsELF = ModuleTriple.isOSBinFormatELF();
  assert((IsELF || IsMSVC) && "Unsupported triple for JMC");
  bool UseX86FastCall = IsMSVC && ModuleTriple.getArch() == Triple::x86;
  const char *const FlagSymbolSection = IsELF ? ".data.just.my.code" : ".msvcjmc";

  GlobalValue *CheckFunction = nullptr;
  DenseMap<DISubprogram *, Constant *> SavedFlags(8);
  // Flag for a file, created once (CodeView symbol __<hash>_<file>).
  auto getFlag = [&](DIFile *File, DICompileUnit *CU) -> Constant * {
    std::string FlagName = getFlagName(*File, UseX86FastCall, IsMSVC);
    IntegerType *FlagTy = Type::getInt8Ty(Ctx);
    return M.getOrInsertGlobal(FlagName, FlagTy, [&] {
      // FIXME: Put the GV in comdat and have linkonce_odr linkage to save
      //        .msvcjmc section space? maybe not worth it.
      GlobalVariable *GV = new GlobalVariable(
          M, FlagTy, /*isConstant=*/false, GlobalValue::InternalLinkage,
          ConstantInt::get(FlagTy, 1), FlagName);
      GV->setSection(FlagSymbolSection);
      GV->setAlignment(Align(1));
      GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
      attachDebugInfo(*GV, CU, File);
      return GV;
    });
  };

  // clang lists the include closure in cl order (CodeGenModule::emitJMCFileList):
  // one flag byte per file, kept alive as cl's are (llvm.used).
  if (!IsELF)
    if (NamedMDNode *Files = M.getNamedMetadata("llvm.msvc.jmc.files")) {
      DICompileUnit *CU = nullptr;
      if (NamedMDNode *CUs = M.getNamedMetadata("llvm.dbg.cu"))
        if (CUs->getNumOperands())
          CU = dyn_cast<DICompileUnit>(CUs->getOperand(0));
      SmallVector<GlobalValue *, 16> Used;
      for (MDNode *N : Files->operands())
        if (auto *File = dyn_cast<DIFile>(N))
          if (auto *GV = dyn_cast<GlobalVariable>(getFlag(File, CU)))
            Used.push_back(GV);
      appendToUsed(M, Used);
    }

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    auto *SP = F.getSubprogram();
    if (!SP || !SP->getFile())
      continue;

    Constant *&Flag = SavedFlags[SP];
    if (!Flag)
      Flag = getFlag(SP->getFile(), SP->getUnit());
  }
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    auto *SP = F.getSubprogram();
    if (!SP || !SP->getFile())
      continue;
    Constant *Flag = SavedFlags[SP];

    if (!CheckFunction) {
      Function *DefaultCheckFunc =
          createDefaultCheckFunction(M, UseX86FastCall);
      if (IsELF) {
        DefaultCheckFunc->setName(CheckFunctionName);
        DefaultCheckFunc->setLinkage(GlobalValue::WeakAnyLinkage);
        CheckFunction = DefaultCheckFunc;
      } else {
        assert(!M.getFunction(CheckFunctionName) &&
               "JMC instrument more than once?");
        auto *CheckFunc = cast<Function>(
            M.getOrInsertFunction(CheckFunctionName, getCheckFunctionType(Ctx))
                .getCallee());
        CheckFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
        CheckFunc->addParamAttr(0, Attribute::NoUndef);
        if (UseX86FastCall) {
          CheckFunc->setCallingConv(CallingConv::X86_FastCall);
          CheckFunc->addParamAttr(0, Attribute::InReg);
        }
        CheckFunction = CheckFunc;

        // No /INCLUDE of the default: cl links without one (probes section 2).
        StringRef DefaultCheckFunctionName = DefaultCheckFunc->getName();
        Comdat *C = M.getOrInsertComdat(DefaultCheckFunctionName);
        C->setSelectionKind(Comdat::Any);
        DefaultCheckFunc->setComdat(C);
        // Add a linker option /alternatename to set the default implementation
        // for the check function.
        // https://devblogs.microsoft.com/oldnewthing/20200731-00/?p=104024
        std::string AltOption = std::string("/alternatename:") +
                                CheckFunctionName + "=" +
                                DefaultCheckFunctionName.str();
        llvm::Metadata *Ops[] = {llvm::MDString::get(Ctx, AltOption)};
        MDTuple *N = MDNode::get(Ctx, Ops);
        M.getOrInsertNamedMetadata("llvm.linker.options")->addOperand(N);
      }
    }
    // FIXME: it would be nice to make CI scheduling boundary, although in
    //        practice it does not matter much.
    auto *CI = CallInst::Create(getCheckFunctionType(Ctx), CheckFunction,
                                {Flag}, "", F.begin()->getFirstInsertionPt());
    CI->addParamAttr(0, Attribute::NoUndef);
    if (UseX86FastCall) {
      CI->setCallingConv(CallingConv::X86_FastCall);
      CI->addParamAttr(0, Attribute::InReg);
    }

    Changed = true;
  }
  return Changed;
}
