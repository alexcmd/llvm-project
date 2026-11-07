//===- TypeLibImport.cpp - MSVC #import type library header generator -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Writes the C++ equivalent of a type library the way cl's #import lays it out
// (layout measured on cl 14.51 output files): <name>.tlh holds forward
// references, smart pointer typedefs, the type library items and optionally
// named GUIDs; <name>.tli holds the inline wrapper implementations.
//===----------------------------------------------------------------------===//

#include "TypeLibImport.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <oleauto.h>
#endif

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <map>
#include <set>

using namespace clang::typelib;

#ifndef _WIN32
bool clang::typelib::generate(const ImportRequest &, ImportResult &R) {
  R.Error = "type library import requires a Windows host";
  return false;
}
bool clang::typelib::importTypeLibrary(const ImportRequest &Q,
                                       ImportResult &R) {
  return generate(Q, R);
}
bool clang::typelib::resolveRegistered(const std::string &, std::string &,
                                       std::string &, std::string &Err) {
  Err = "type library import requires a Windows host";
  return false;
}
#else

namespace {

std::string narrow(const wchar_t *S) {
  if (!S || !*S)
    return std::string();
  int N = WideCharToMultiByte(CP_UTF8, 0, S, -1, nullptr, 0, nullptr, nullptr);
  std::string R(N > 0 ? N - 1 : 0, '\0');
  if (N > 1)
    WideCharToMultiByte(CP_UTF8, 0, S, -1, &R[0], N, nullptr, nullptr);
  return R;
}

std::string bstr(BSTR B) {
  std::string R = narrow(B);
  SysFreeString(B);
  return R;
}

std::string fmt(const char *F, ...) {
  char Buf[512];
  va_list A;
  va_start(A, F);
  vsnprintf(Buf, sizeof(Buf), F, A);
  va_end(A);
  return Buf;
}

bool isNullGuid(const GUID &G) { return G == GUID_NULL; }

// "bef6e003-a874-101a-8bba-00aa00300cab"
std::string guidText(const GUID &G) {
  return fmt("%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", G.Data1,
             G.Data2, G.Data3, G.Data4[0], G.Data4[1], G.Data4[2], G.Data4[3],
             G.Data4[4], G.Data4[5], G.Data4[6], G.Data4[7]);
}

// "{0xbef6e003,0xa874,0x101a,{0x8b,0xba,0x00,0xaa,0x00,0x30,0x0c,0xab}}"
std::string guidInit(const GUID &G) {
  return fmt("{0x%08lx,0x%04x,0x%04x,{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%"
             "02x,0x%02x,0x%02x}}",
             G.Data1, G.Data2, G.Data3, G.Data4[0], G.Data4[1], G.Data4[2],
             G.Data4[3], G.Data4[4], G.Data4[5], G.Data4[6], G.Data4[7]);
}

struct Item;
struct TypeClass;
struct Wrapper;
struct WrapperSection;

struct Func {
  FUNCDESC *FD = nullptr;
  std::vector<std::string> Names; // [0] = function, then params
};

struct Item {
  unsigned Index = 0;
  ITypeInfo *TI = nullptr;  // as listed in the library
  TYPEATTR *TA = nullptr;
  ITypeInfo *VTI = nullptr; // vtable side of a dual interface
  TYPEATTR *VTA = nullptr;
  std::string Name;         // after rename()
  TYPEKIND Kind = TKIND_MAX;
  bool Dual = false;
  bool Excluded = false;
  bool Emitted = false;
  std::vector<Item *> Deps;
};

// Types cl leaves out by default (no_auto_exclude keeps them): they are
// already declared by the Windows SDK headers that comdef.h pulls in.
const char *const AutoExcluded[] = {
    "IUnknown", "IDispatch", "IEnumVARIANT", "IFont", "IPicture", "_FILETIME",
    "ISequentialStream",
};

class Generator {
public:
  Generator(const ImportRequest &Req) : Req(Req), O(Req.Options) {}
  ~Generator();
  bool run(ImportResult &R);

private:
  const ImportRequest &Req;
  const ImportOptions &O;
  ITypeLib *TL = nullptr;
  TLIBATTR *LA = nullptr;
  std::string LibName, NS;
  std::vector<Item> Items;
  std::map<std::string, Item *> ByName;
  std::vector<Item *> Order; // emission order of the type library items
  std::string H;             // .tlh text (LF newlines; CRLF on output)
  std::string I;             // .tli text

  std::string rename(const std::string &N) const;
  std::string typeName(ITypeInfo *TI);
  Item *findItem(ITypeInfo *TI);
  bool isExcludedName(const std::string &N) const;
  std::string refTypeText(ITypeInfo *Ctx, HREFTYPE H);
  std::string typeText(ITypeInfo *Ctx, const TYPEDESC &TD);
  void collectDeps(Item &It);
  void computeOrder();
  void loadFuncs(ITypeInfo *TI, TYPEATTR *TA, std::vector<Func> &Out);
  void freeFuncs(ITypeInfo *TI, std::vector<Func> &Fs);
  std::string varValueText(const VARIANT &V, ITypeInfo *Ctx,
                           const TYPEDESC &TD);

  void emitForward();
  void emitSmartPointers();
  void emitItems();
  void emitEnum(Item &It);
  void emitRecord(Item &It);
  void emitCoclass(Item &It);
  void emitInterface(Item &It);
  void emitDispinterface(Item &It);
  void emitModule(Item &It);
  void emitRawMethods(ITypeInfo *TI, std::vector<Func> &Fs,
                      const std::vector<bool> &HasWrapper);
  TypeClass classify(ITypeInfo *Ctx, const TYPEDESC &TD);
  std::string memberName(const Func &F, int Kind);
  void buildVtableWrappers(Item &It, ITypeInfo *TI, TYPEATTR *TA,
                           std::vector<Func> &Fs, std::vector<Wrapper> &Ws,
                           std::vector<bool> &HasWrapper);
  void emitVtableProperties(ITypeInfo *TI, std::vector<Func> &Fs);
  std::vector<WrapperSection> Sections;
  size_t KeyedThreshold = 1000;
  std::string qualify(const TypeClass &C, const std::string &T, bool Void);
  void emitImplementations(const std::string &TliPath);
  void emitWrapperDecls(const std::vector<Wrapper> &Ws);
  void emitWrapperDefs(const std::string &Kind, const Item &It,
                       const std::vector<Wrapper> &Ws);
  std::string paramName(const Func &F, int P);
  void emitNamedGuids();
};

Generator::~Generator() {
  for (Item &It : Items) {
    if (It.VTI) {
      It.VTI->ReleaseTypeAttr(It.VTA);
      It.VTI->Release();
    }
    if (It.TI) {
      It.TI->ReleaseTypeAttr(It.TA);
      It.TI->Release();
    }
  }
  if (TL) {
    if (LA)
      TL->ReleaseTLibAttr(LA);
    TL->Release();
  }
}

std::string Generator::rename(const std::string &N) const {
  for (const auto &P : O.Renames)
    if (P.first == N)
      return P.second;
  if (O.AutoRename && Req.IsMacro && Req.IsMacro(N))
    return "__" + N;
  return N;
}

bool Generator::isExcludedName(const std::string &N) const {
  for (const std::string &E : O.Exclude)
    if (E == N)
      return true;
  if (!O.NoAutoExclude)
    for (const char *E : AutoExcluded)
      if (N == E)
        return true;
  return false;
}

std::string Generator::typeName(ITypeInfo *TI) {
  BSTR N = nullptr;
  TI->GetDocumentation(MEMBERID_NIL, &N, nullptr, nullptr, nullptr);
  return rename(bstr(N));
}

Item *Generator::findItem(ITypeInfo *TI) {
  ITypeLib *Lib = nullptr;
  UINT Idx = 0;
  if (FAILED(TI->GetContainingTypeLib(&Lib, &Idx)))
    return nullptr;
  bool Same = Lib == TL;
  if (!Same) {
    TLIBATTR *A = nullptr;
    if (SUCCEEDED(Lib->GetLibAttr(&A))) {
      Same = A->guid == LA->guid && A->wMajorVerNum == LA->wMajorVerNum &&
             A->wMinorVerNum == LA->wMinorVerNum && A->lcid == LA->lcid;
      Lib->ReleaseTLibAttr(A);
    }
  }
  Lib->Release();
  if (!Same || Idx >= Items.size())
    return nullptr;
  return &Items[Idx];
}

// Spelling of a reference to a user-defined type.
std::string Generator::refTypeText(ITypeInfo *Ctx, HREFTYPE HRef) {
  ITypeInfo *R = nullptr;
  if (FAILED(Ctx->GetRefTypeInfo(HRef, &R)))
    return "void";
  std::string N = typeName(R);
  TYPEATTR *A = nullptr;
  R->GetTypeAttr(&A);
  TYPEKIND K = A->typekind;
  std::string Out;
  switch (K) {
  case TKIND_ENUM:
    Out = "enum " + N;
    break;
  case TKIND_RECORD:
    // The OLE records come from the SDK headers under their typedef names.
    if (N == "GUID" || N == "DISPPARAMS" || N == "EXCEPINFO")
      Out = N;
    else
      Out = "struct " + N;
    break;
  case TKIND_UNION:
    Out = "union " + N;
    break;
  case TKIND_ALIAS:
    Out = N;
    break;
  case TKIND_INTERFACE:
  case TKIND_DISPATCH:
    if (N == "IUnknown" || N == "IDispatch")
      Out = N;
    else
      Out = "struct " + N;
    break;
  case TKIND_COCLASS: {
    // A coclass stands for its default interface.
    Out = "struct " + N;
    for (int J = 0; J < A->cImplTypes; ++J) {
      INT F = 0;
      R->GetImplTypeFlags(J, &F);
      if ((F & IMPLTYPEFLAG_FDEFAULT) && !(F & IMPLTYPEFLAG_FSOURCE)) {
        HREFTYPE H2;
        ITypeInfo *D = nullptr;
        if (SUCCEEDED(R->GetRefTypeOfImplType(J, &H2)) &&
            SUCCEEDED(R->GetRefTypeInfo(H2, &D))) {
          std::string DN = typeName(D);
          D->Release();
          Out = (DN == "IUnknown" || DN == "IDispatch") ? DN : "struct " + DN;
        }
        break;
      }
    }
    break;
  }
  default:
    Out = N;
  }
  R->ReleaseTypeAttr(A);
  R->Release();
  return Out;
}

std::string Generator::typeText(ITypeInfo *Ctx, const TYPEDESC &TD) {
  switch (TD.vt) {
  case VT_PTR:
    return typeText(Ctx, *TD.lptdesc) + " *";
  case VT_SAFEARRAY:
    return "SAFEARRAY *";
  case VT_CARRAY:
    return typeText(Ctx, TD.lpadesc->tdescElem);
  case VT_USERDEFINED:
    return refTypeText(Ctx, TD.hreftype);
  case VT_I2: return "short";
  case VT_I4: return "long";
  case VT_R4: return "float";
  case VT_R8: return "double";
  case VT_CY: return "CURRENCY";
  case VT_DATE: return "DATE";
  case VT_BSTR: return "BSTR";
  case VT_DISPATCH: return "IDispatch *";
  case VT_ERROR: return "SCODE";
  case VT_BOOL: return "VARIANT_BOOL";
  case VT_VARIANT: return "VARIANT";
  case VT_UNKNOWN: return "IUnknown *";
  case VT_DECIMAL: return "DECIMAL";
  case VT_I1: return "char";
  case VT_UI1: return "unsigned char";
  case VT_UI2: return "unsigned short";
  case VT_UI4: return "unsigned long";
  case VT_I8: return "__int64";
  case VT_UI8: return "unsigned __int64";
  case VT_INT: return "int";
  case VT_UINT: return "unsigned int";
  case VT_VOID: return "void";
  case VT_HRESULT: return "HRESULT";
  case VT_LPSTR: return "LPSTR";
  case VT_LPWSTR: return "LPWSTR";
  case VT_INT_PTR: return "INT_PTR";
  case VT_UINT_PTR: return "UINT_PTR";
  default:
    return fmt("/* vt %d */ void", TD.vt);
  }
}

// Array suffix of a declarator ("[8]").
std::string arraySuffix(const TYPEDESC &TD) {
  if (TD.vt != VT_CARRAY)
    return std::string();
  std::string S;
  for (USHORT D = 0; D < TD.lpadesc->cDims; ++D)
    S += fmt("[%lu]", TD.lpadesc->rgbounds[D].cElements);
  return S;
}

void Generator::loadFuncs(ITypeInfo *TI, TYPEATTR *TA, std::vector<Func> &Out) {
  for (WORD J = 0; J < TA->cFuncs; ++J) {
    Func F;
    if (FAILED(TI->GetFuncDesc(J, &F.FD)))
      continue;
    std::vector<BSTR> Names(F.FD->cParams + 1);
    UINT N = 0;
    TI->GetNames(F.FD->memid, Names.data(), (UINT)Names.size(), &N);
    for (UINT K = 0; K < Names.size(); ++K)
      F.Names.push_back(K < N ? bstr(Names[K]) : std::string());
    Out.push_back(std::move(F));
  }
}

void Generator::freeFuncs(ITypeInfo *TI, std::vector<Func> &Fs) {
  for (Func &F : Fs)
    TI->ReleaseFuncDesc(F.FD);
  Fs.clear();
}

void addUdtDeps(ITypeInfo *Ctx, const TYPEDESC &TD,
                std::vector<std::pair<ITypeInfo *, HREFTYPE>> &Out) {
  const TYPEDESC *T = &TD;
  while (T->vt == VT_PTR || T->vt == VT_SAFEARRAY)
    T = T->lptdesc;
  if (T->vt == VT_CARRAY)
    T = &T->lpadesc->tdescElem;
  if (T->vt == VT_USERDEFINED)
    Out.push_back({Ctx, T->hreftype});
}

// Items an item's definition waits for: base interface, types named in its
// own methods (interfaces and dual interfaces only) and record fields.
// Aliases, dispinterface members, coclasses and enums add none.
void Generator::collectDeps(Item &It) {
  ITypeInfo *TI = It.VTI ? It.VTI : It.TI;
  TYPEATTR *TA = It.VTI ? It.VTA : It.TA;
  std::vector<std::pair<ITypeInfo *, HREFTYPE>> Refs;
  bool Methods = It.Kind == TKIND_INTERFACE || It.Dual;
  if (It.Kind == TKIND_RECORD || It.Kind == TKIND_UNION) {
    for (WORD J = 0; J < TA->cVars; ++J) {
      VARDESC *VD;
      if (SUCCEEDED(TI->GetVarDesc(J, &VD))) {
        addUdtDeps(TI, VD->elemdescVar.tdesc, Refs);
        TI->ReleaseVarDesc(VD);
      }
    }
  } else if (Methods) {
    for (WORD J = 0; J < TA->cImplTypes; ++J) {
      HREFTYPE H;
      if (SUCCEEDED(TI->GetRefTypeOfImplType(J, &H)))
        Refs.push_back({TI, H});
    }
    for (WORD J = 0; J < TA->cFuncs; ++J) {
      FUNCDESC *FD;
      if (FAILED(TI->GetFuncDesc(J, &FD)))
        continue;
      addUdtDeps(TI, FD->elemdescFunc.tdesc, Refs);
      for (SHORT P = 0; P < FD->cParams; ++P)
        addUdtDeps(TI, FD->lprgelemdescParam[P].tdesc, Refs);
      TI->ReleaseFuncDesc(FD);
    }
  }
  for (auto &R : Refs) {
    ITypeInfo *RT = nullptr;
    if (FAILED(R.first->GetRefTypeInfo(R.second, &RT)))
      continue;
    Item *D = findItem(RT);
    RT->Release();
    if (!D || D == &It || D->Excluded || D->Kind == TKIND_ALIAS)
      continue;
    if (std::find(It.Deps.begin(), It.Deps.end(), D) == It.Deps.end())
      It.Deps.push_back(D);
  }
}

// cl's order: repeatedly take the first item (library order) whose
// dependencies are all emitted; on a cycle force the first item whose base
// interface is already emitted.
void Generator::computeOrder() {
  std::vector<Item *> Rem;
  for (Item &It : Items)
    if (!It.Excluded && It.Kind != TKIND_ALIAS) {
      collectDeps(It);
      Rem.push_back(&It);
    }
  auto baseOf = [&](Item *It) -> Item * {
    if (It->Kind != TKIND_INTERFACE && !It->Dual)
      return nullptr;
    ITypeInfo *TI = It->VTI ? It->VTI : It->TI;
    TYPEATTR *TA = It->VTI ? It->VTA : It->TA;
    if (TA->cImplTypes < 1)
      return nullptr;
    HREFTYPE H;
    ITypeInfo *B = nullptr;
    if (FAILED(TI->GetRefTypeOfImplType(0, &H)) ||
        FAILED(TI->GetRefTypeInfo(H, &B)))
      return nullptr;
    Item *R = findItem(B);
    B->Release();
    return R && !R->Excluded ? R : nullptr;
  };
  while (!Rem.empty()) {
    auto Ready = Rem.end();
    for (auto It = Rem.begin(); It != Rem.end(); ++It) {
      bool OK = true;
      for (Item *D : (*It)->Deps)
        if (!D->Emitted) {
          OK = false;
          break;
        }
      if (OK) {
        Ready = It;
        break;
      }
    }
    if (Ready == Rem.end()) {
      for (auto It = Rem.begin(); It != Rem.end(); ++It) {
        Item *B = baseOf(*It);
        if (!B || B->Emitted) {
          Ready = It;
          break;
        }
      }
      if (Ready == Rem.end())
        Ready = Rem.begin();
    }
    (*Ready)->Emitted = true;
    Order.push_back(*Ready);
    Rem.erase(Ready);
  }
}

const char *forwardComment(const Item &It) {
  if (It.Kind == TKIND_INTERFACE)
    return "interface";
  return It.Dual ? "dual interface" : "dispinterface";
}

void Generator::emitForward() {
  H += "//\n// Forward references and typedefs\n//\n\n";
  H += "struct __declspec(uuid(\"" + guidText(LA->guid) + "\"))\n/* LIBID */ __" +
       LibName + ";\n";
  for (Item &It : Items) {
    if (It.Excluded)
      continue;
    switch (It.Kind) {
    case TKIND_ENUM:
      H += "enum " + It.Name + " : int;\n";
      break;
    case TKIND_RECORD:
      H += "struct " + It.Name + ";\n";
      break;
    case TKIND_UNION:
      H += "union " + It.Name + ";\n";
      break;
    case TKIND_INTERFACE:
    case TKIND_DISPATCH:
      H += "struct __declspec(uuid(\"" + guidText(It.TA->guid) + "\"))\n/* " +
           forwardComment(It) + " */ " + It.Name + ";\n";
      break;
    case TKIND_COCLASS:
      H += "struct /* coclass */ " + It.Name + ";\n";
      break;
    default:
      break;
    }
  }
  for (Item &It : Items) {
    if (It.Excluded || It.Kind != TKIND_ALIAS)
      continue;
    const TYPEDESC &TD = It.TA->tdescAlias;
    if (It.Name == "ULONG_PTR" || It.Name == "LONG_PTR") {
      bool U = It.Name[0] == 'U';
      H += "#if !defined(_WIN64)\n";
      H += std::string("typedef __w64 ") + (U ? "unsigned long" : "long") +
           " " + It.Name + ";\n";
      H += "#else\n";
      H += std::string("typedef ") + (U ? "unsigned __int64" : "__int64") +
           " " + It.Name + ";\n";
      H += "#endif\n";
      continue;
    }
    H += "typedef " + typeText(It.TI, TD) + " " + It.Name + arraySuffix(TD) +
         ";\n";
  }
}

void Generator::emitSmartPointers() {
  H += "\n//\n// Smart pointer typedef declarations\n//\n\n";
  for (Item *It : Order)
    if (It->Kind == TKIND_INTERFACE || It->Kind == TKIND_DISPATCH)
      H += "_COM_SMARTPTR_TYPEDEF(" + It->Name + ", __uuidof(" + It->Name +
           "));\n";
}

std::string Generator::varValueText(const VARIANT &V, ITypeInfo *Ctx,
                                    const TYPEDESC &TD) {
  switch (V.vt) {
  case VT_BSTR: {
    std::string S = narrow(V.bstrVal), R;
    for (char C : S) {
      if (C == '"' || C == '\\')
        R += '\\';
      R += C;
    }
    return "(char*) \"" + R + "\"";
  }
  case VT_I1: return fmt("%d", V.cVal);
  case VT_UI1: return fmt("%u", V.bVal);
  case VT_I2: return fmt("%d", V.iVal);
  case VT_UI2: return fmt("%u", V.uiVal);
  case VT_I4: return fmt("%ld", V.lVal);
  case VT_UI4: return fmt("%lu", V.ulVal);
  case VT_INT: return fmt("%d", V.intVal);
  case VT_UINT: return fmt("%u", V.uintVal);
  case VT_I8: return fmt("%lld", V.llVal);
  case VT_UI8: return fmt("%llu", V.ullVal);
  case VT_R4: return fmt("%g", V.fltVal);
  case VT_R8: return fmt("%g", V.dblVal);
  case VT_BOOL: return fmt("%d", V.boolVal);
  default: {
    VARIANT X;
    VariantInit(&X);
    std::string R = "0";
    if (SUCCEEDED(VariantChangeType(&X, const_cast<VARIANT *>(&V), 0, VT_BSTR)))
      R = narrow(X.bstrVal);
    VariantClear(&X);
    return R;
  }
  }
}

void Generator::emitEnum(Item &It) {
  H += "\n";
  if (isNullGuid(It.TA->guid))
    H += "enum " + It.Name + " : int\n{\n";
  else
    H += "enum __declspec(uuid(\"" + guidText(It.TA->guid) + "\"))\n" +
         It.Name + " : int\n{\n";
  for (WORD J = 0; J < It.TA->cVars; ++J) {
    VARDESC *VD;
    if (FAILED(It.TI->GetVarDesc(J, &VD)))
      continue;
    BSTR N = nullptr;
    UINT C = 0;
    It.TI->GetNames(VD->memid, &N, 1, &C);
    H += "    " + rename(bstr(N)) + " = " +
         varValueText(*VD->lpvarValue, It.TI, VD->elemdescVar.tdesc) +
         (J + 1 < It.TA->cVars ? ",\n" : "\n");
    It.TI->ReleaseVarDesc(VD);
  }
  H += "};\n";
}

void Generator::emitRecord(Item &It) {
  H += fmt("\n#pragma pack(push, %u)\n\n", (unsigned)It.TA->cbAlignment);
  H += std::string(It.Kind == TKIND_UNION ? "union " : "struct ") + It.Name +
       "\n{\n";
  for (WORD J = 0; J < It.TA->cVars; ++J) {
    VARDESC *VD;
    if (FAILED(It.TI->GetVarDesc(J, &VD)))
      continue;
    BSTR N = nullptr;
    UINT C = 0;
    It.TI->GetNames(VD->memid, &N, 1, &C);
    const TYPEDESC &TD = VD->elemdescVar.tdesc;
    H += "    " + typeText(It.TI, TD) + " " + rename(bstr(N)) +
         arraySuffix(TD) + ";\n";
    It.TI->ReleaseVarDesc(VD);
  }
  H += "};\n\n#pragma pack(pop)\n";
}

void Generator::emitCoclass(Item &It) {
  H += "\n";
  H += "struct __declspec(uuid(\"" + guidText(It.TA->guid) + "\"))\n" +
       It.Name + ";\n";
  for (WORD J = 0; J < It.TA->cImplTypes; ++J) {
    HREFTYPE HR;
    ITypeInfo *R = nullptr;
    INT F = 0;
    if (FAILED(It.TI->GetRefTypeOfImplType(J, &HR)) ||
        FAILED(It.TI->GetRefTypeInfo(HR, &R)))
      continue;
    It.TI->GetImplTypeFlags(J, &F);
    std::string N = typeName(R);
    TYPEATTR *A;
    R->GetTypeAttr(&A);
    bool Disp = A->typekind == TKIND_DISPATCH &&
                !(A->wTypeFlags & TYPEFLAG_FDUAL);
    R->ReleaseTypeAttr(A);
    R->Release();
    std::vector<const char *> Fl;
    if (F & IMPLTYPEFLAG_FDEFAULT)
      Fl.push_back("default");
    if (F & IMPLTYPEFLAG_FSOURCE)
      Fl.push_back("source");
    if (F & IMPLTYPEFLAG_FRESTRICTED)
      Fl.push_back("restricted");
    if (F & IMPLTYPEFLAG_FDEFAULTVTABLE)
      Fl.push_back("defaultvtable");
    std::string L = "    // ";
    if (!Fl.empty()) {
      L += "[ ";
      for (size_t K = 0; K < Fl.size(); ++K)
        L += std::string(K ? ", " : "") + Fl[K];
      L += " ] ";
    }
    H += L + (Disp ? "dispinterface " : "interface ") + N + "\n";
  }
}

std::string paramAnnotation(USHORT F) {
  std::vector<const char *> A;
  if (F & PARAMFLAG_FIN)
    A.push_back("in");
  if (F & PARAMFLAG_FOUT)
    A.push_back("out");
  if (F & PARAMFLAG_FLCID)
    A.push_back("lcid");
  if (F & PARAMFLAG_FRETVAL)
    A.push_back("retval");
  if (A.empty())
    return std::string();
  std::string S = "/*[";
  for (size_t K = 0; K < A.size(); ++K)
    S += std::string(K ? "," : "") + A[K];
  return S + "]*/ ";
}

const char *callConv(CALLCONV C) {
  switch (C) {
  case CC_CDECL: return "__cdecl";
  case CC_PASCAL: return "__pascal";
  case CC_FASTCALL: return "__fastcall";
  default: return "__stdcall";
  }
}

// Unnamed parameters are spelled _arg<position>.
std::string Generator::paramName(const Func &F, int P) {
  std::string N = (size_t)P + 1 < F.Names.size() ? F.Names[P + 1] : "";
  return N.empty() ? fmt("_arg%d", P + 1) : rename(N);
}

bool isMissingVariant(const ELEMDESC &E) {
  USHORT F = E.paramdesc.wParamFlags;
  return (F & PARAMFLAG_FOPT) && !(F & PARAMFLAG_FHASDEFAULT) &&
         E.tdesc.vt == VT_VARIANT;
}

// The trailing run of [optional] VARIANTs without [defaultvalue] defaults to
// vtMissing.
std::string optionalDefault(const FUNCDESC *FD, int P, bool SkipRetval = false) {
  int End = FD->cParams;
  if (SkipRetval && End > 0 &&
      (FD->lprgelemdescParam[End - 1].paramdesc.wParamFlags & PARAMFLAG_FRETVAL))
    --End;
  for (int Q = P; Q < End; ++Q)
    if (!isMissingVariant(FD->lprgelemdescParam[Q]))
      return std::string();
  return " = vtMissing";
}

void Generator::emitRawMethods(ITypeInfo *TI, std::vector<Func> &Fs,
                               const std::vector<bool> &HasWrapper) {
  H += "    //\n    // Raw methods provided by interface\n    //\n\n";
  for (size_t J = 0; J < Fs.size(); ++J) {
    Func &F = Fs[J];
    FUNCDESC *FD = F.FD;
    std::string N = rename(F.Names[0]);
    if (FD->invkind & INVOKE_PROPERTYGET)
      N = O.RawPropGet + N;
    else if (FD->invkind & INVOKE_PROPERTYPUT)
      N = O.RawPropPut + N;
    else if (FD->invkind & INVOKE_PROPERTYPUTREF)
      N = O.RawPropPutRef + N;
    else if (HasWrapper[J])
      N = O.RawMethodPrefix + N;
    H += "      virtual " + typeText(TI, FD->elemdescFunc.tdesc) + " " +
         callConv(FD->callconv) + " " + N + " (";
    if (FD->cParams == 0) {
      H += " ) = 0;\n";
      continue;
    }
    H += "\n";
    for (SHORT P = 0; P < FD->cParams; ++P) {
      const ELEMDESC &E = FD->lprgelemdescParam[P];
      H += "        " + paramAnnotation(E.paramdesc.wParamFlags) +
           typeText(TI, E.tdesc) + " " + paramName(F, P) +
           arraySuffix(E.tdesc) + optionalDefault(FD, P) +
           (P + 1 < FD->cParams ? ",\n" : " ) = 0;\n");
    }
  }
}

// Shape of a type for the wrapper layer.
struct TypeClass {
  enum Kind { Void, Hresult, Bstr, Variant, IfacePtr, Enum, Scalar, Pointer,
              Other } K = Other;
  std::string Raw;  // raw spelling
  std::string Ptr;  // smart pointer typedef for IfacePtr
  bool Alias = false;
  bool LibType = false; // names a type of this library (keyed tli qualifies)
  VARTYPE VT = VT_EMPTY; // for _com_dispatch_* calls
};

TypeClass Generator::classify(ITypeInfo *Ctx, const TYPEDESC &TD) {
  TypeClass C;
  C.Raw = typeText(Ctx, TD);
  switch (TD.vt) {
  case VT_VOID: C.K = TypeClass::Void; C.VT = VT_EMPTY; return C;
  case VT_HRESULT: C.K = TypeClass::Hresult; C.VT = VT_ERROR; return C;
  case VT_BSTR: C.K = TypeClass::Bstr; C.VT = VT_BSTR; return C;
  case VT_VARIANT: C.K = TypeClass::Variant; C.VT = VT_VARIANT; return C;
  case VT_DISPATCH:
    C.K = TypeClass::IfacePtr; C.Ptr = "IDispatchPtr"; C.VT = VT_DISPATCH;
    return C;
  case VT_UNKNOWN:
    C.K = TypeClass::IfacePtr; C.Ptr = "IUnknownPtr"; C.VT = VT_UNKNOWN;
    return C;
  case VT_INT: C.K = TypeClass::Scalar; C.VT = VT_I4; return C;
  case VT_UINT: C.K = TypeClass::Scalar; C.VT = VT_UI4; return C;
  case VT_CY: C.K = TypeClass::Other; C.VT = VT_CY; return C;
  case VT_DECIMAL: C.K = TypeClass::Other; C.VT = VT_DECIMAL; return C;
  case VT_SAFEARRAY:
    C.K = TypeClass::Pointer;
    C.VT = VT_ARRAY | (TD.lptdesc->vt == VT_USERDEFINED ? VT_VARIANT
                                                        : TD.lptdesc->vt);
    return C;
  case VT_PTR: {
    const TYPEDESC &P = *TD.lptdesc;
    if (P.vt == VT_USERDEFINED) {
      ITypeInfo *R = nullptr;
      if (SUCCEEDED(Ctx->GetRefTypeInfo(P.hreftype, &R))) {
        TYPEATTR *A;
        R->GetTypeAttr(&A);
        TYPEKIND K = A->typekind;
        bool Disp = K == TKIND_DISPATCH ||
                    (A->wTypeFlags & (TYPEFLAG_FDUAL | TYPEFLAG_FDISPATCHABLE));
        R->ReleaseTypeAttr(A);
        if (K == TKIND_INTERFACE || K == TKIND_DISPATCH ||
            K == TKIND_COCLASS) {
          std::string N = C.Raw.substr(0, C.Raw.size() - 2); // drop " *"
          if (N.compare(0, 7, "struct ") == 0)
            N = N.substr(7);
          C.K = TypeClass::IfacePtr;
          C.Ptr = N + "Ptr";
          C.LibType = findItem(R) != nullptr;
          C.VT = Disp ? VT_DISPATCH : VT_UNKNOWN;
          R->Release();
          return C;
        }
        R->Release();
      }
    }
    TypeClass In = classify(Ctx, P);
    C.K = TypeClass::Pointer;
    C.VT = VT_BYREF | In.VT;
    return C;
  }
  case VT_USERDEFINED: {
    ITypeInfo *R = nullptr;
    if (FAILED(Ctx->GetRefTypeInfo(TD.hreftype, &R)))
      return C;
    TYPEATTR *A;
    R->GetTypeAttr(&A);
    C.LibType = findItem(R) != nullptr;
    if (A->typekind == TKIND_ENUM) {
      C.K = TypeClass::Enum;
      C.VT = VT_I4;
    } else if (A->typekind == TKIND_ALIAS) {
      TypeClass U = classify(R, A->tdescAlias);
      C.K = U.K == TypeClass::Bstr || U.K == TypeClass::Variant ? U.K
                                                              : TypeClass::Other;
      C.Alias = true;
      C.VT = U.VT;
    } else if (A->typekind == TKIND_RECORD) {
      C.VT = VT_RECORD;
    } else if (A->typekind == TKIND_INTERFACE ||
               A->typekind == TKIND_DISPATCH) {
      C.VT = A->typekind == TKIND_DISPATCH ? VT_DISPATCH : VT_UNKNOWN;
    }
    R->ReleaseTypeAttr(A);
    R->Release();
    return C;
  }
  default:
    C.K = TypeClass::Scalar;
    C.VT = TD.vt;
    return C;
  }
}

std::string highParam(const TypeClass &C) {
  if (C.Alias)
    return C.Raw;
  if (C.K == TypeClass::Bstr)
    return "_bstr_t";
  if (C.K == TypeClass::Variant)
    return "const _variant_t &";
  if (C.K == TypeClass::Other)
    return "const " + C.Raw + " &"; // CURRENCY, DECIMAL, records by value
  return C.Raw;
}

std::string highRet(const TypeClass &C) {
  if (C.Alias)
    return C.Raw;
  switch (C.K) {
  case TypeClass::Bstr: return "_bstr_t";
  case TypeClass::Variant: return "_variant_t";
  case TypeClass::IfacePtr: return C.Ptr;
  default: return C.Raw;
  }
}

// "    T _result = 0;" style declaration of the out value.
std::string resultDecl(const TypeClass &C) {
  if (C.Alias)
    return "    " + C.Raw + " _result;\n";
  switch (C.K) {
  case TypeClass::Variant:
    return "    VARIANT _result;\n    VariantInit(&_result);\n";
  case TypeClass::Bstr:
  case TypeClass::IfacePtr:
  case TypeClass::Scalar:
  case TypeClass::Pointer:
  case TypeClass::Hresult:
    return "    " + C.Raw + " _result = 0;\n";
  default:
    return "    " + C.Raw + " _result;\n";
  }
}

std::string resultExpr(const TypeClass &C) {
  if (C.Alias)
    return "_result";
  switch (C.K) {
  case TypeClass::Bstr: return "_bstr_t(_result, false)";
  case TypeClass::Variant: return "_variant_t(_result, false)";
  case TypeClass::IfacePtr: return C.Ptr + "(_result, false)";
  default: return "_result";
  }
}

// Stock DISPIDs are spelled by name.
std::string dispidText(MEMBERID Id) {
  static const struct { long Id; const char *N; } Stock[] = {
      {-500, "DISPID_AUTOSIZE"}, {-501, "DISPID_BACKCOLOR"},
      {-502, "DISPID_BACKSTYLE"}, {-503, "DISPID_BORDERCOLOR"},
      {-504, "DISPID_BORDERSTYLE"}, {-505, "DISPID_BORDERWIDTH"},
      {-507, "DISPID_DRAWMODE"}, {-508, "DISPID_DRAWSTYLE"},
      {-509, "DISPID_DRAWWIDTH"}, {-510, "DISPID_FILLCOLOR"},
      {-511, "DISPID_FILLSTYLE"}, {-512, "DISPID_FONT"},
      {-513, "DISPID_FORECOLOR"}, {-514, "DISPID_ENABLED"},
      {-515, "DISPID_HWND"}, {-516, "DISPID_TABSTOP"},
      {-517, "DISPID_TEXT"}, {-518, "DISPID_CAPTION"},
      {-519, "DISPID_BORDERVISIBLE"}, {-520, "DISPID_APPEARANCE"},
      {-521, "DISPID_MOUSEPOINTER"}, {-522, "DISPID_MOUSEICON"},
      {-523, "DISPID_PICTURE"}, {-524, "DISPID_VALID"},
      {-525, "DISPID_READYSTATE"}, {-550, "DISPID_REFRESH"},
      {-551, "DISPID_DOCLICK"}, {-552, "DISPID_ABOUTBOX"},
      {-553, "DISPID_ADDITEM"}, {-554, "DISPID_CLEAR"},
      {-555, "DISPID_REMOVEITEM"}, {-600, "DISPID_CLICK"},
      {-601, "DISPID_DBLCLICK"}, {-602, "DISPID_KEYDOWN"},
      {-603, "DISPID_KEYPRESS"}, {-604, "DISPID_KEYUP"},
      {-605, "DISPID_MOUSEDOWN"}, {-606, "DISPID_MOUSEMOVE"},
      {-607, "DISPID_MOUSEUP"}, {-608, "DISPID_ERROREVENT"},
      {-609, "DISPID_READYSTATECHANGE"},
  };
  for (const auto &S : Stock)
    if (S.Id == (long)Id)
      return S.N;
  return fmt("0x%lx", (unsigned long)Id);
}

const char *vtName(VARTYPE V) {
  switch (V & VT_TYPEMASK) {
  case VT_EMPTY: return "VT_EMPTY";
  case VT_I2: return "VT_I2";
  case VT_I4: return "VT_I4";
  case VT_R4: return "VT_R4";
  case VT_R8: return "VT_R8";
  case VT_CY: return "VT_CY";
  case VT_DATE: return "VT_DATE";
  case VT_BSTR: return "VT_BSTR";
  case VT_DISPATCH: return "VT_DISPATCH";
  case VT_ERROR: return "VT_ERROR";
  case VT_BOOL: return "VT_BOOL";
  case VT_VARIANT: return "VT_VARIANT";
  case VT_UNKNOWN: return "VT_UNKNOWN";
  case VT_DECIMAL: return "VT_DECIMAL";
  case VT_I1: return "VT_I1";
  case VT_UI1: return "VT_UI1";
  case VT_UI2: return "VT_UI2";
  case VT_UI4: return "VT_UI4";
  case VT_I8: return "VT_I8";
  case VT_UI8: return "VT_UI8";
  case VT_INT: return "VT_INT";
  case VT_UINT: return "VT_UINT";
  case VT_RECORD: return "VT_RECORD";
  default: return "VT_EMPTY";
  }
}

std::string Generator::memberName(const Func &F, int Kind) {
  std::string N = rename(F.Names[0]);
  (void)Kind;
  return N;
}

// One wrapper (vtable interface) or dispatch wrapper.
struct Wrapper {
  std::string Name;   // high-level name
  std::string Raw;    // raw method it calls (vtable wrappers)
  std::string Ret;    // high-level return type
  std::vector<std::string> DeclParams; // "type name[ = default]"
  std::vector<std::string> DefParams;  // "type name"
  std::string Body;   // tli body (between the braces)
  bool LibRet = false;
  std::string QRet;   // return type qualified with the namespace (keyed tli)
};

struct WrapperSection {
  std::string Kind, Item;
  std::vector<Wrapper> Ws;
};

void Generator::buildVtableWrappers(Item &It, ITypeInfo *TI, TYPEATTR *TA,
                                    std::vector<Func> &Fs,
                                    std::vector<Wrapper> &Ws,
                                    std::vector<bool> &HasWrapper) {
  HasWrapper.assign(Fs.size(), false);
  for (size_t J = 0; J < Fs.size(); ++J) {
    Func &F = Fs[J];
    FUNCDESC *FD = F.FD;
    if (FD->elemdescFunc.tdesc.vt != VT_HRESULT)
      continue;
    HasWrapper[J] = true;
    Wrapper W;
    std::string N = rename(F.Names[0]);
    bool Get = FD->invkind & INVOKE_PROPERTYGET,
         Put = FD->invkind & INVOKE_PROPERTYPUT,
         PutRef = FD->invkind & INVOKE_PROPERTYPUTREF;
    W.Name = Get ? O.HighPropGet + N
             : Put ? O.HighPropPut + N
             : PutRef ? O.HighPropPutRef + N
             : O.HighMethodPrefix + N;
    W.Raw = Get ? O.RawPropGet + N
            : Put ? O.RawPropPut + N
            : PutRef ? O.RawPropPutRef + N
            : O.RawMethodPrefix + N;
    int NP = FD->cParams;
    int RetIdx = -1;
    if (NP > 0 && (FD->lprgelemdescParam[NP - 1].paramdesc.wParamFlags &
                   PARAMFLAG_FRETVAL))
      RetIdx = NP - 1;
    TypeClass RC;
    if (RetIdx >= 0) {
      RC = classify(TI, *FD->lprgelemdescParam[RetIdx].tdesc.lptdesc);
      W.Ret = highRet(RC);
      W.QRet = qualify(RC, W.Ret, false);
    } else
      W.QRet = W.Ret = (Put || PutRef) ? "void" : "HRESULT";
    std::string Args;
    for (int P = 0; P < NP; ++P) {
      if (P == RetIdx)
        continue;
      const ELEMDESC &E = FD->lprgelemdescParam[P];
      TypeClass PC = classify(TI, E.tdesc);
      std::string PN = paramName(F, P);
      std::string T = highParam(PC);
      W.DefParams.push_back(T + " " + PN);
      W.DeclParams.push_back(T + " " + PN + optionalDefault(FD, P, true));
      Args += (Args.empty() ? "" : ", ") + PN;
    }
    std::string B;
    if (RetIdx >= 0) {
      B += resultDecl(RC);
      Args += std::string(Args.empty() ? "" : ", ") + "&_result";
    }
    B += "    HRESULT _hr = " + W.Raw + "(" + Args + ");\n";
    B += "    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));\n";
    if (RetIdx >= 0)
      B += "    return " + resultExpr(RC) + ";\n";
    else if (W.Ret == "HRESULT")
      B += "    return _hr;\n";
    W.Body = B;
    Ws.push_back(std::move(W));
  }
}

std::string repeat(const char *S, int N) {
  std::string R;
  while (N-- > 0)
    R += S;
  return R;
}

// Property data of a vtable interface: get/put/putref accessors by name.
void Generator::emitVtableProperties(ITypeInfo *TI, std::vector<Func> &Fs) {
  struct Prop {
    std::string Name, Get, Put, Type;
    MEMBERID Id;
    bool HasPut = false, HasPutRef = false, Mismatch = false;
    int Indexed = 0;
  };
  std::vector<Prop> Ps;
  for (Func &F : Fs) {
    FUNCDESC *FD = F.FD;
    if (FD->elemdescFunc.tdesc.vt != VT_HRESULT ||
        !(FD->invkind & (INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT |
                         INVOKE_PROPERTYPUTREF)))
      continue;
    std::string N = rename(F.Names[0]);
    Prop *P = nullptr;
    for (Prop &Q : Ps)
      if (Q.Name == N)
        P = &Q;
    if (!P) {
      Ps.push_back(Prop());
      P = &Ps.back();
      P->Name = N;
      P->Id = FD->memid;
    }
    int NP = FD->cParams;
    // An accessor with an argument that is neither [in] nor [out,retval]
    // keeps the whole property out of the property data.
    for (int Q = 0; Q < NP; ++Q)
      if (!(FD->lprgelemdescParam[Q].paramdesc.wParamFlags &
            (PARAMFLAG_FIN | PARAMFLAG_FRETVAL)))
        P->Mismatch = true;
    if (FD->invkind & INVOKE_PROPERTYGET) {
      P->Get = O.HighPropGet + N;
      bool RV = NP > 0 && (FD->lprgelemdescParam[NP - 1].paramdesc.wParamFlags &
                           PARAMFLAG_FRETVAL);
      if (RV) {
        TypeClass C = classify(TI, *FD->lprgelemdescParam[NP - 1].tdesc.lptdesc);
        if (!P->Type.empty() && P->Type != highRet(C))
          P->Mismatch = true;
        P->Type = highRet(C);
        if (NP > 1)
          P->Indexed = NP - 1;
      }
    } else {
      if (FD->invkind & INVOKE_PROPERTYPUT) {
        P->HasPut = true;
        P->Put = O.HighPropPut + N;
      } else {
        P->HasPutRef = true;
        P->Put = O.HighPropPutRef + N;
      }
      if (NP > 1)
        P->Indexed = NP - 1;
      if (NP > 0) {
        TypeClass C = classify(TI, FD->lprgelemdescParam[NP - 1].tdesc);
        if (P->Type.empty())
          P->Type = highRet(C);
        else if (P->Type != highRet(C))
          P->Mismatch = true;
      }
    }
  }
  std::vector<Prop *> Out;
  for (Prop &P : Ps)
    if (!P.Mismatch)
      Out.push_back(&P);
  std::stable_sort(Out.begin(), Out.end(), [](Prop *A, Prop *B) {
    return (unsigned long)A->Id < (unsigned long)B->Id;
  });
  if (Out.empty())
    return;
  H += "    //\n    // Property data\n    //\n\n";
  for (Prop *P : Out) {
    std::string D = "    __declspec(property(";
    if (!P->Get.empty())
      D += "get=" + P->Get;
    if (!P->Put.empty())
      D += std::string(P->Get.empty() ? "" : ",") + "put=" + P->Put;
    H += D + "))\n    " + P->Type + " " + P->Name +
         repeat("[]", P->Indexed) + ";\n";
  }
  H += "\n";
}

void Generator::emitWrapperDecls(const std::vector<Wrapper> &Ws) {
  for (const Wrapper &W : Ws) {
    H += "    " + W.Ret + " " + W.Name + " (";
    if (W.DeclParams.empty()) {
      H += " );\n";
      continue;
    }
    H += "\n";
    for (size_t K = 0; K < W.DeclParams.size(); ++K)
      H += "        " + W.DeclParams[K] +
           (K + 1 < W.DeclParams.size() ? ",\n" : " );\n");
  }
}

void Generator::emitWrapperDefs(const std::string &Kind, const Item &It,
                                const std::vector<Wrapper> &Ws) {
  if (!Ws.empty())
    Sections.push_back({Kind, It.Name, Ws});
}

// Namespace-qualified spelling of a library type (keyed .tli).
std::string Generator::qualify(const TypeClass &C, const std::string &T,
                               bool Void) {
  if (Void || !C.LibType || O.NoNamespace)
    return T;
  if (T.compare(0, 5, "enum ") == 0)
    return "enum " + NS + "::" + T.substr(5);
  if (T.compare(0, 7, "struct ") == 0)
    return "struct " + NS + "::" + T.substr(7);
  if (T.compare(0, 6, "union ") == 0)
    return "union " + NS + "::" + T.substr(6);
  return NS + "::" + T;
}

// Wrapper definitions: one #include'd .tli, or (large libraries) cl's
// implementation_key map region with fully qualified definitions.
void Generator::emitImplementations(const std::string &TliPath) {
  size_t Count = 0;
  for (const WrapperSection &S : Sections)
    Count += S.Ws.size();
  bool Keyed = Count > KeyedThreshold;
  std::string Q = O.NoNamespace ? "" : NS + "::";
  unsigned Key = 0;
  std::string Map;
  for (const WrapperSection &S : Sections) {
    I += "\n//\n// " + S.Kind + " " + S.Item +
         " wrapper method implementations\n//\n";
    for (const Wrapper &W : S.Ws) {
      std::string Params;
      for (size_t K = 0; K < W.DefParams.size(); ++K)
        Params += (K ? ", " : "") + W.DefParams[K];
      std::string Sig = " ( " + Params + (Params.empty() ? ")" : " )");
      if (Keyed) {
        ++Key;
        I += fmt("\n#pragma implementation_key(%u)\n", Key) + "inline " +
             W.QRet + " " + Q + S.Item + "::" + W.Name + Sig + " {\n" +
             W.Body + "}\n";
        Map += fmt("__declspec(implementation_key(%u)) ", Key) + W.Ret + " " +
               S.Item + "::" + W.Name + Sig + ";\n";
      } else {
        I += "\ninline " + W.Ret + " " + S.Item + "::" + W.Name + Sig +
             " {\n" + W.Body + "}\n";
      }
    }
  }
  if (Keyed) {
    H += "\n//\n// Function implementation mapping\n//\n\n#pragma "
         "start_map_region(\"" +
         TliPath + "\")\n" + Map + "#pragma stop_map_region\n";
  } else {
    H += "\n//\n// Wrapper method implementations\n//\n\n#include \"" +
         TliPath + "\"\n";
    I = "#pragma once\n" + I;
  }
}

void Generator::emitInterface(Item &It) {
  ITypeInfo *TI = It.VTI ? It.VTI : It.TI;
  TYPEATTR *TA = It.VTI ? It.VTA : It.TA;
  std::string Base;
  if (TA->cImplTypes > 0) {
    HREFTYPE HR;
    ITypeInfo *B = nullptr;
    if (SUCCEEDED(TI->GetRefTypeOfImplType(0, &HR)) &&
        SUCCEEDED(TI->GetRefTypeInfo(HR, &B))) {
      Base = typeName(B);
      B->Release();
    }
  }
  H += "\nstruct __declspec(uuid(\"" + guidText(It.TA->guid) + "\"))\n" +
       It.Name + (Base.empty() ? "" : " : " + Base) + "\n";
  if (TA->cFuncs == 0) {
    H += "{};\n";
    return;
  }
  H += "{\n";
  std::vector<Func> Fs;
  loadFuncs(TI, TA, Fs);
  std::vector<bool> HasWrapper(Fs.size(), false);
  if (!O.RawInterfacesOnly) {
    std::vector<Wrapper> Ws;
    buildVtableWrappers(It, TI, TA, Fs, Ws, HasWrapper);
    emitVtableProperties(TI, Fs);
    if (!Ws.empty()) {
      H += "    //\n    // Wrapper methods for error-handling\n    //\n\n";
      emitWrapperDecls(Ws);
      H += "\n";
    }
    emitWrapperDefs("interface", It, Ws);
  }
  emitRawMethods(TI, Fs, HasWrapper);
  freeFuncs(TI, Fs);
  H += "};\n";
}

void Generator::emitDispinterface(Item &It) {
  H += "\nstruct __declspec(uuid(\"" + guidText(It.TA->guid) + "\"))\n" +
       It.Name + " : IDispatch\n";
  if (O.RawInterfacesOnly || (It.TA->cFuncs == 0 && It.TA->cVars == 0)) {
    H += "{};\n";
    return;
  }
  H += "{\n";
  ITypeInfo *TI = It.TI;
  // Property data: dispinterface properties.
  std::vector<VARDESC *> Vs;
  std::vector<std::string> VNames;
  for (WORD J = 0; J < It.TA->cVars; ++J) {
    VARDESC *VD;
    if (FAILED(TI->GetVarDesc(J, &VD)))
      continue;
    BSTR N = nullptr;
    UINT C = 0;
    TI->GetNames(VD->memid, &N, 1, &C);
    Vs.push_back(VD);
    VNames.push_back(rename(bstr(N)));
  }
  if (!Vs.empty()) {
    H += "    //\n    // Property data\n    //\n\n";
    for (size_t J = 0; J < Vs.size(); ++J) {
      TypeClass C = classify(TI, Vs[J]->elemdescVar.tdesc);
      bool RO = Vs[J]->wVarFlags & VARFLAG_FREADONLY;
      H += "    __declspec(property(get=" + O.HighPropGet + VNames[J] +
           (RO ? "" : ",put=" + O.HighPropPut + VNames[J]) + "))\n    " +
           highRet(C) + " " + VNames[J] + ";\n";
    }
    H += "\n";
  }
  H += "    //\n    // Wrapper methods for error-handling\n    //\n\n";
  std::vector<Func> Fs;
  loadFuncs(TI, It.TA, Fs);
  std::vector<Wrapper> DWs, PWs;
  if (!Fs.empty()) {
    H += "    // Methods:\n";
    for (Func &F : Fs) {
      FUNCDESC *FD = F.FD;
      std::string N = rename(F.Names[0]);
      // Arguments the dispatch helpers cannot pass.
      std::string Bad;
      for (SHORT P = 0; P < FD->cParams && Bad.empty(); ++P) {
        const TYPEDESC &T = FD->lprgelemdescParam[P].tdesc;
        if (T.vt == VT_PTR && T.lptdesc->vt == VT_VOID)
          Bad = "Method '" + N + "' not emitted because argument '" +
                paramName(F, P) + "' has invalid type: " + typeText(TI, T);
      }
      if (!Bad.empty()) {
        H += "    // " + Bad + "\n";
        continue;
      }
      TypeClass RC = classify(TI, FD->elemdescFunc.tdesc);
      bool VoidRet = RC.K == TypeClass::Void;
      std::string Ret = VoidRet ? "HRESULT" : highRet(RC);
      std::string Decl = "    " + Ret + " " + O.HighMethodPrefix + N + " (";
      std::string Params, Fmt, Args;
      std::vector<std::string> DP;
      for (SHORT P = 0; P < FD->cParams; ++P) {
        const ELEMDESC &E = FD->lprgelemdescParam[P];
        TypeClass PC = classify(TI, E.tdesc);
        std::string PN = paramName(F, P);
        std::string T = highParam(PC);
        DP.push_back(T + " " + PN + optionalDefault(FD, P));
        Params += (P ? ", " : "") + T + " " + PN;
        Fmt += fmt("\\x%04x", (unsigned)PC.VT);
        std::string A = PN;
        if (PC.K == TypeClass::Bstr && !PC.Alias)
          A = "(BSTR)" + PN;
        else if (PC.K == TypeClass::Variant && !PC.Alias)
          A = "&" + PN;
        Args += ", " + A;
      }
      if (DP.empty())
        Decl += " );\n";
      else {
        Decl += "\n";
        for (size_t K = 0; K < DP.size(); ++K)
          Decl += "        " + DP[K] + (K + 1 < DP.size() ? ",\n" : " );\n");
      }
      H += Decl;
      std::string Call = "_com_dispatch_method(this, " + dispidText(FD->memid) +
                         ", DISPATCH_METHOD, ";
      std::string Tail = FD->cParams ? " \n        L\"" + Fmt + "\"" + Args + ");\n"
                                     : " NULL);\n";
      std::string B;
      if (VoidRet)
        B = "    return " + Call + "VT_EMPTY, NULL," + Tail;
      else
        B = resultDecl(RC) + "    " + Call + vtName(RC.VT) +
            ", (void*)&_result," + Tail + "    return " + resultExpr(RC) +
            ";\n";
      {
        Wrapper W;
        W.Name = O.HighMethodPrefix + N;
        W.Ret = Ret;
        W.QRet = qualify(RC, Ret, VoidRet);
        W.Body = B;
        for (SHORT P = 0; P < FD->cParams; ++P)
          W.DefParams.push_back(
              highParam(classify(TI, FD->lprgelemdescParam[P].tdesc)) + " " +
              paramName(F, P));
        DWs.push_back(std::move(W));
      }
    }
  }
  if (!Vs.empty()) {
    if (!Fs.empty())
      H += "\n";
    H += "    // Properties:\n";
    for (size_t J = 0; J < Vs.size(); ++J) {
      TypeClass C = classify(TI, Vs[J]->elemdescVar.tdesc);
      bool RO = Vs[J]->wVarFlags & VARFLAG_FREADONLY;
      std::string R = highRet(C);
      std::string PT = C.K == TypeClass::Other && !C.Alias
                           ? "const " + C.Raw + " &"
                           : highParam(C);
      std::string G = O.HighPropGet + VNames[J], P = O.HighPropPut + VNames[J];
      H += "    " + R + " " + G + " ( );\n";
      unsigned long Id = (unsigned long)Vs[J]->memid;
      {
        Wrapper W;
        W.Name = G;
        W.Ret = R;
        W.QRet = qualify(C, R, false);
        W.Body = resultDecl(C) + "    _com_dispatch_propget(this, " +
                 dispidText(Id) + ", " + vtName(C.VT) + ", (void*)&_result);\n" +
                 "    return " + resultExpr(C) + ";\n";
        PWs.push_back(std::move(W));
      }
      if (!RO) {
        H += "    void " + P + " ( " + PT + " _val );\n";
        std::string A = "_val";
        if (C.K == TypeClass::Bstr && !C.Alias)
          A = "(BSTR)_val";
        else if ((C.K == TypeClass::Variant && !C.Alias) ||
                 C.K == TypeClass::Other && !C.Alias)
          A = "&_val";
        Wrapper W;
        W.Name = P;
        W.Ret = W.QRet = "void";
        W.DefParams.push_back(PT + " _val");
        W.Body = "    _com_dispatch_propput(this, " + dispidText(Id) + ", " +
                 vtName(C.VT) + ", " + A + ");\n";
        PWs.push_back(std::move(W));
      }
    }
  }
  freeFuncs(TI, Fs);
  for (VARDESC *VD : Vs)
    TI->ReleaseVarDesc(VD);
  H += "};\n";
  DWs.insert(DWs.end(), PWs.begin(), PWs.end());
  if (!DWs.empty())
    Sections.push_back({"dispinterface", It.Name, std::move(DWs)});
}

void Generator::emitModule(Item &It) {
  std::vector<Func> Fs;
  loadFuncs(It.TI, It.TA, Fs);
  for (Func &F : Fs) {
    FUNCDESC *FD = F.FD;
    H += "    " + typeText(It.TI, FD->elemdescFunc.tdesc) + " " +
         callConv(FD->callconv) + " " + rename(F.Names[0]) + "(";
    if (FD->cParams == 0) {
      H += " );\n";
      continue;
    }
    H += "\n";
    for (SHORT P = 0; P < FD->cParams; ++P) {
      const ELEMDESC &E = FD->lprgelemdescParam[P];
      std::string PN = (size_t)P + 1 < F.Names.size() ? F.Names[P + 1] : "";
      H += "        " + typeText(It.TI, E.tdesc) + " " + rename(PN) +
           arraySuffix(E.tdesc) + (P + 1 < FD->cParams ? ",\n" : " );\n");
    }
  }
  freeFuncs(It.TI, Fs);
  for (WORD J = 0; J < It.TA->cVars; ++J) {
    VARDESC *VD;
    if (FAILED(It.TI->GetVarDesc(J, &VD)))
      continue;
    BSTR N = nullptr;
    UINT C = 0;
    It.TI->GetNames(VD->memid, &N, 1, &C);
    const TYPEDESC &TD = VD->elemdescVar.tdesc;
    if (VD->varkind == VAR_CONST)
      H += "    const " + typeText(It.TI, TD) + " " + rename(bstr(N)) + " = " +
           varValueText(*VD->lpvarValue, It.TI, TD) + ";\n";
    else
      SysFreeString(N);
    It.TI->ReleaseVarDesc(VD);
  }
}

void Generator::emitItems() {
  H += "\n//\n// Type library items\n//\n";
  for (Item *It : Order) {
    switch (It->Kind) {
    case TKIND_ENUM:
      emitEnum(*It);
      break;
    case TKIND_RECORD:
    case TKIND_UNION:
      emitRecord(*It);
      break;
    case TKIND_COCLASS:
      emitCoclass(*It);
      break;
    case TKIND_INTERFACE:
      emitInterface(*It);
      break;
    case TKIND_DISPATCH:
      if (It->Dual)
        emitInterface(*It);
      else
        emitDispinterface(*It);
      break;
    case TKIND_MODULE:
      emitModule(*It);
      break;
    default:
      break;
    }
  }
}

void Generator::emitNamedGuids() {
  H += "\n//\n// Named GUID constants initializations\n//\n\n";
  H += "extern \"C\" const GUID __declspec(selectany) LIBID_" + LibName +
       " =\n    " + guidInit(LA->guid) + ";\n";
  for (Item *It : Order) {
    const char *P = nullptr;
    if (It->Kind == TKIND_COCLASS)
      P = "CLSID_";
    else if (It->Kind == TKIND_INTERFACE || It->Dual)
      P = "IID_";
    else if (It->Kind == TKIND_DISPATCH)
      P = "DIID_";
    if (!P)
      continue;
    H += std::string("extern \"C\" const GUID __declspec(selectany) ") + P +
         It->Name + " =\n    " + guidInit(It->TA->guid) + ";\n";
  }
}

std::string toCRLF(const std::string &S) {
  std::string R = "\xEF\xBB\xBF";
  R.reserve(S.size() + S.size() / 16 + 3);
  for (char C : S) {
    if (C == '\n')
      R += '\r';
    R += C;
  }
  return R;
}

bool Generator::run(ImportResult &R) {
  HRESULT HR = LoadTypeLibEx(Req.TypeLibPath.c_str(), REGKIND_NONE, &TL);
  if (FAILED(HR)) {
    R.Error = fmt("cannot load type library (hr 0x%08lx)", (unsigned long)HR);
    return false;
  }
  TL->GetLibAttr(&LA);
  BSTR LN = nullptr;
  TL->GetDocumentation(MEMBERID_NIL, &LN, nullptr, nullptr, nullptr);
  LibName = bstr(LN);
  NS = O.HasRenameNamespace ? O.RenameNamespace : LibName;

  UINT N = TL->GetTypeInfoCount();
  Items.resize(N);
  for (UINT J = 0; J < N; ++J) {
    Item &It = Items[J];
    It.Index = J;
    if (FAILED(TL->GetTypeInfo(J, &It.TI))) {
      It.Excluded = true;
      continue;
    }
    It.TI->GetTypeAttr(&It.TA);
    It.Kind = It.TA->typekind;
    It.Name = typeName(It.TI);
    It.Dual = It.Kind == TKIND_DISPATCH &&
              (It.TA->wTypeFlags & TYPEFLAG_FDUAL) && !O.NoDualInterfaces;
    if (It.Dual) {
      HREFTYPE HRef;
      if (SUCCEEDED(It.TI->GetRefTypeOfImplType(-1, &HRef)) &&
          SUCCEEDED(It.TI->GetRefTypeInfo(HRef, &It.VTI)))
        It.VTI->GetTypeAttr(&It.VTA);
      else
        It.Dual = false;
    }
    BSTR Orig = nullptr;
    It.TI->GetDocumentation(MEMBERID_NIL, &Orig, nullptr, nullptr, nullptr);
    It.Excluded = isExcludedName(bstr(Orig));
    ByName[It.Name] = &It;
  }
  computeOrder();

  std::string TlhPath = Req.OutputDir + "\\" + Req.BaseName + ".tlh";
  std::string TliPath = Req.OutputDir + "\\" + Req.BaseName + ".tli";
  const char *Kind =
      LA->syskind == SYS_WIN32 ? "Win32 type library" : "type library";

  H = "// " + Req.Banner + "\n//\n// " + TlhPath + "\n//\n" +
      "// C++ source equivalent of " + Kind + " " + Req.SpelledName +
      "\n// compiler-generated file - DO NOT EDIT!\n\n";
  H += "#pragma once\n#pragma pack(push, 8)\n\n#include <comdef.h>\n\n";
  if (!O.NoNamespace)
    H += "namespace " + NS + " {\n\n";
  emitForward();
  if (!O.NoSmartPointers)
    emitSmartPointers();
  emitItems();
  if (O.NamedGuids)
    emitNamedGuids();
  bool WantTli = !O.RawInterfacesOnly && !O.NoImplementation;
  if (WantTli || O.ImplementationOnly) {
    std::string Tlh = H;
    emitImplementations(TliPath);
    if (!WantTli)
      H = Tlh;
  }
  if (!O.NoNamespace)
    H += "\n} // namespace " + NS + "\n";
  H += "\n#pragma pack(pop)\n";

  R.TlhPath = TlhPath;
  R.TlhText = toCRLF(H);
  if (WantTli || O.ImplementationOnly) {
    R.TliPath = TliPath;
    std::string T = "// " + Req.Banner + "\n//\n// " + TliPath + "\n//\n" +
                    "// Wrapper implementations for " + Kind + " " +
                    Req.SpelledName +
                    "\n// compiler-generated file - DO NOT EDIT!\n\n";
    R.TliText = toCRLF(T + I);
  }
  return true;
}

} // namespace

bool clang::typelib::generate(const ImportRequest &Req, ImportResult &R) {
  HRESULT CI = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  bool OK;
  {
    Generator G(Req);
    OK = G.run(R);
  }
  if (SUCCEEDED(CI))
    CoUninitialize();
  return OK;
}

#endif

namespace {
// Attribute list scanner: name [ "(" "str" {"," "str"} ")" ] separated by
// whitespace or commas.
struct AttrScanner {
  const std::string &S;
  size_t P = 0;
  void ws() {
    while (P < S.size() && (isspace((unsigned char)S[P]) || S[P] == ','))
      ++P;
  }
};
} // namespace

bool clang::typelib::parseAttributes(const std::string &S, ImportOptions &O,
                                      std::string &Err) {
  AttrScanner Sc{S};
  while (true) {
    Sc.ws();
    if (Sc.P >= S.size())
      return true;
    size_t B = Sc.P;
    while (Sc.P < S.size() &&
           (isalnum((unsigned char)S[Sc.P]) || S[Sc.P] == '_'))
      ++Sc.P;
    std::string Name = S.substr(B, Sc.P - B);
    if (Name.empty()) {
      Err = "unexpected character in #import attribute list";
      return false;
    }
    std::vector<std::string> Args;
    bool HasArgs = false;
    while (Sc.P < S.size() && isspace((unsigned char)S[Sc.P]))
      ++Sc.P;
    if (Sc.P < S.size() && S[Sc.P] == '(') {
      HasArgs = true;
      ++Sc.P;
      while (true) {
        while (Sc.P < S.size() &&
               (isspace((unsigned char)S[Sc.P]) || S[Sc.P] == ','))
          ++Sc.P;
        if (Sc.P >= S.size()) {
          Err = "missing ')' in #import attribute '" + Name + "'";
          return false;
        }
        if (S[Sc.P] == ')') {
          ++Sc.P;
          break;
        }
        if (S[Sc.P] == '"') {
          std::string A;
          ++Sc.P;
          while (Sc.P < S.size() && S[Sc.P] != '"') {
            if (S[Sc.P] == '\\' && Sc.P + 1 < S.size())
              ++Sc.P;
            A += S[Sc.P++];
          }
          ++Sc.P;
          Args.push_back(A);
        } else {
          size_t AB = Sc.P;
          while (Sc.P < S.size() && S[Sc.P] != ',' && S[Sc.P] != ')' &&
                 !isspace((unsigned char)S[Sc.P]))
            ++Sc.P;
          Args.push_back(S.substr(AB, Sc.P - AB));
        }
      }
    }
    auto need = [&](size_t Min) {
      if (Args.size() < Min) {
        Err = "#import attribute '" + Name + "' needs an argument";
        return false;
      }
      return true;
    };
    if (Name == "raw_interfaces_only") O.RawInterfacesOnly = true;
    else if (Name == "named_guids") O.NamedGuids = true;
    else if (Name == "no_namespace") O.NoNamespace = true;
    else if (Name == "rename_namespace") {
      if (!need(1)) return false;
      O.HasRenameNamespace = true;
      O.RenameNamespace = Args[0];
    } else if (Name == "no_implementation") O.NoImplementation = true;
    else if (Name == "implementation_only") O.ImplementationOnly = true;
    else if (Name == "raw_native_types") O.RawNativeTypes = true;
    else if (Name == "no_smart_pointers") O.NoSmartPointers = true;
    else if (Name == "raw_dispinterfaces") O.RawDispinterfaces = true;
    else if (Name == "no_dual_interfaces") O.NoDualInterfaces = true;
    else if (Name == "no_auto_exclude") O.NoAutoExclude = true;
    else if (Name == "auto_rename") O.AutoRename = true;
    else if (Name == "no_search_namespace") O.NoSearchNamespace = true;
    else if (Name == "embedded_idl") O.EmbeddedIdl = true;
    else if (Name == "raw_method_prefix") {
      if (!need(1)) return false;
      O.HasRawMethodPrefix = true;
      O.RawMethodPrefix = Args[0];
    } else if (Name == "high_method_prefix") {
      if (!need(1)) return false;
      O.HasHighMethodPrefix = true;
      O.HighMethodPrefix = Args[0];
    } else if (Name == "raw_property_prefixes") {
      if (!need(3)) return false;
      O.RawPropGet = Args[0]; O.RawPropPut = Args[1]; O.RawPropPutRef = Args[2];
    } else if (Name == "high_property_prefixes") {
      if (!need(3)) return false;
      O.HighPropGet = Args[0]; O.HighPropPut = Args[1]; O.HighPropPutRef = Args[2];
    } else if (Name == "exclude") {
      if (!need(1)) return false;
      O.Exclude.insert(O.Exclude.end(), Args.begin(), Args.end());
    } else if (Name == "rename") {
      if (!need(2)) return false;
      O.Renames.push_back({Args[0], Args[1]});
    } else if (Name == "inject_statement") {
      if (!need(1)) return false;
      O.InjectStatements.push_back(Args[0]);
    } else if (Name == "auto_search" || Name == "include" ||
               Name == "rename_search_namespace" || Name == "tlbid" ||
               Name == "no_registry") {
      (void)HasArgs; // accepted; resolution-only attributes
    } else {
      Err = "unknown #import attribute '" + Name + "'";
      return false;
    }
  }
}

#ifdef _WIN32
namespace {
std::wstring widen(const std::string &S) {
  int N = MultiByteToWideChar(CP_UTF8, 0, S.c_str(), -1, nullptr, 0);
  std::wstring R(N > 0 ? N - 1 : 0, L'\0');
  if (N > 1)
    MultiByteToWideChar(CP_UTF8, 0, S.c_str(), -1, &R[0], N);
  return R;
}

bool fileTime(const std::wstring &P, FILETIME &T) {
  WIN32_FILE_ATTRIBUTE_DATA D;
  if (!GetFileAttributesExW(P.c_str(), GetFileExInfoStandard, &D))
    return false;
  T = D.ftLastWriteTime;
  return true;
}

bool readFirstLine(const std::wstring &P, std::string &L) {
  FILE *F = _wfopen(P.c_str(), L"rb");
  if (!F)
    return false;
  char Buf[1024];
  size_t N = fread(Buf, 1, sizeof(Buf), F);
  fclose(F);
  std::string S(Buf, N);
  size_t E = S.find("\r\n");
  L = S.substr(0, E);
  return E != std::string::npos;
}

bool writeStamped(const std::string &P, const std::string &Text,
                  const FILETIME &T) {
  std::wstring W = widen(P);
  SetFileAttributesW(W.c_str(), FILE_ATTRIBUTE_NORMAL);
  HANDLE Hd = CreateFileW(W.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL, nullptr);
  if (Hd == INVALID_HANDLE_VALUE)
    return false;
  DWORD Wr = 0;
  bool OK = WriteFile(Hd, Text.data(), (DWORD)Text.size(), &Wr, nullptr) &&
            Wr == Text.size();
  SetFileTime(Hd, nullptr, nullptr, &T);
  CloseHandle(Hd);
  return OK;
}

uint32_t fnv1a(const std::string &S) {
  uint32_t H = 2166136261u;
  for (unsigned char C : S)
    H = (H ^ C) * 16777619u;
  return H;
}
} // namespace

bool clang::typelib::resolveRegistered(const std::string &Spec,
                                       std::string &Path, std::string &Display,
                                       std::string &Err) {
  HRESULT CI = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  bool OK = false;
  GUID Lib = GUID_NULL;
  WORD Maj = 0xFFFF, Min = 0xFFFF;
  std::string Rest;
  if (Spec.compare(0, 7, "progid:") == 0) {
    Rest = Spec.substr(7);
    Display = Rest;
    CLSID C;
    if (SUCCEEDED(CLSIDFromProgID(widen(Rest).c_str(), &C))) {
      wchar_t Key[128], Val[64];
      wchar_t CS[64];
      StringFromGUID2(C, CS, 64);
      swprintf(Key, 128, L"CLSID\\%s\\TypeLib", CS);
      LONG Sz = sizeof(Val);
      if (RegQueryValueW(HKEY_CLASSES_ROOT, Key, Val, &Sz) == ERROR_SUCCESS)
        CLSIDFromString(Val, &Lib);
    }
  } else if (Spec.compare(0, 6, "libid:") == 0) {
    Rest = Spec.substr(6);
    Display = Rest;
    std::string G = Rest, V;
    size_t Comma = Rest.find(',');
    if (Comma != std::string::npos) {
      G = Rest.substr(0, Comma);
      V = Rest.substr(Comma + 1);
    }
    while (!G.empty() && isspace((unsigned char)G.back()))
      G.pop_back();
    if (G.empty() || G[0] != '{')
      G = "{" + G + "}";
    CLSIDFromString(widen(G).c_str(), &Lib);
    unsigned A, B;
    if (!V.empty() && sscanf(V.c_str(), "%u.%u", &A, &B) == 2) {
      Maj = (WORD)A;
      Min = (WORD)B;
    }
  }
  if (Lib != GUID_NULL) {
    if (Maj == 0xFFFF) {
      // Highest registered version.
      wchar_t GS[64], Key[128];
      StringFromGUID2(Lib, GS, 64);
      swprintf(Key, 128, L"TypeLib\\%s", GS);
      HKEY K;
      if (RegOpenKeyExW(HKEY_CLASSES_ROOT, Key, 0, KEY_READ, &K) ==
          ERROR_SUCCESS) {
        wchar_t Sub[64];
        unsigned BestA = 0, BestB = 0;
        bool Any = false;
        for (DWORD J = 0; RegEnumKeyW(K, J, Sub, 64) == ERROR_SUCCESS; ++J) {
          unsigned A = 0, B = 0;
          if (swscanf(Sub, L"%x.%x", &A, &B) >= 1 &&
              (!Any || A > BestA || (A == BestA && B > BestB))) {
            BestA = A;
            BestB = B;
            Any = true;
          }
        }
        RegCloseKey(K);
        if (Any) {
          Maj = (WORD)BestA;
          Min = (WORD)BestB;
        }
      }
    }
    BSTR P = nullptr;
    if (Maj != 0xFFFF && SUCCEEDED(QueryPathOfRegTypeLib(Lib, Maj, Min,
                                                         LOCALE_NEUTRAL, &P))) {
      Path = bstr(P);
      while (!Path.empty() && Path.back() == '\0')
        Path.pop_back();
      OK = true;
    }
  }
  if (!OK)
    Err = "type library for '" + Spec + "' is not registered";
  if (SUCCEEDED(CI))
    CoUninitialize();
  return OK;
}

bool clang::typelib::importTypeLibrary(const ImportRequest &Req0,
                                       ImportResult &R) {
  ImportRequest Req = Req0;
  char HB[16];
  snprintf(HB, sizeof(HB), "%08x",
           fnv1a(Req.AttrText + "\n" + Req.SpelledName + "\n" +
                 narrow(Req.TypeLibPath.c_str()) + "\n" + Req.Banner));
  Req.Banner += std::string(" (") + HB + ").";
  const ImportOptions &O = Req.Options;
  R.TlhPath = Req.OutputDir + "\\" + Req.BaseName + ".tlh";
  bool NeedTli = !O.RawInterfacesOnly && !O.NoImplementation;
  if (NeedTli)
    R.TliPath = Req.OutputDir + "\\" + Req.BaseName + ".tli";
  FILETIME LibT, T;
  std::wstring LibFile = Req.TypeLibPath;
  if (!fileTime(LibFile, LibT)) {
    R.Error = "cannot open type library file";
    return false;
  }
  std::string First;
  bool Fresh =
      fileTime(widen(R.TlhPath), T) && CompareFileTime(&T, &LibT) == 0 &&
      readFirstLine(widen(R.TlhPath), First) &&
      First == "\xEF\xBB\xBF// " + Req.Banner &&
      (!NeedTli || (fileTime(widen(R.TliPath), T) &&
                    CompareFileTime(&T, &LibT) == 0));
  if (Fresh)
    return true;
  if (!generate(Req, R))
    return false;
  if (!O.ImplementationOnly) {
    if (!writeStamped(R.TlhPath, R.TlhText, LibT)) {
      R.Error = "cannot write '" + R.TlhPath + "'";
      return false;
    }
    R.WroteTlh = true;
  }
  if (!R.TliPath.empty() && (NeedTli || O.ImplementationOnly)) {
    if (!writeStamped(R.TliPath, R.TliText, LibT)) {
      R.Error = "cannot write '" + R.TliPath + "'";
      return false;
    }
    R.WroteTli = true;
  }
  return true;
}
#endif
