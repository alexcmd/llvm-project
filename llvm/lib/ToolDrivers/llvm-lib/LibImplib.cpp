//===- LibImplib.cpp - link /LIB /DEF import library from objects ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LibImplib.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/Object/COFF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FileOutputBuffer.h"
#include "llvm/Support/StringSaver.h"
#include <algorithm>
#include <string>

using namespace llvm;
using namespace llvm::object;

static Error specError(StringRef Spec) {
  return createStringError(inconvertibleErrorCode(),
                           "invalid /EXPORT: '" + Spec + "'");
}

Error libimplib::parseExportSpec(StringRef Spec, COFFShortExport &E) {
  E = COFFShortExport();
  auto [Names, Rest] = Spec.split(',');
  auto [Entry, Internal] = Names.split('=');
  if (Entry.empty())
    return specError(Spec);
  if (!Internal.empty()) {
    E.Name = Internal.str();
    E.ExtName = Entry.str();
  } else {
    E.Name = Entry.str();
  }
  while (!Rest.empty()) {
    StringRef Tok;
    std::tie(Tok, Rest) = Rest.split(',');
    if (Tok.equals_insensitive("NONAME")) {
      E.Noname = true;
    } else if (Tok.equals_insensitive("DATA")) {
      E.Data = true;
    } else if (Tok.equals_insensitive("CONSTANT")) {
      E.Constant = true;
    } else if (Tok.equals_insensitive("PRIVATE")) {
      E.Private = true;
    } else if (Tok.starts_with("@")) {
      unsigned Ord;
      if (Tok.drop_front().getAsInteger(0, Ord) || Ord == 0 || Ord > 0xFFFF)
        return specError(Spec);
      E.Ordinal = Ord;
    } else {
      return specError(Spec);
    }
  }
  return Error::success();
}

Error libimplib::collectDirectiveExports(
    MemoryBufferRef MB, std::vector<COFFShortExport> &Exports) {
  if (identify_magic(MB.getBuffer()) != file_magic::coff_object)
    return Error::success();
  Expected<std::unique_ptr<COFFObjectFile>> Obj =
      COFFObjectFile::create(MB);
  if (!Obj)
    return Obj.takeError();
  BumpPtrAllocator Alloc;
  StringSaver Saver(Alloc);
  for (const SectionRef &S : (*Obj)->sections()) {
    Expected<StringRef> Name = S.getName();
    if (!Name)
      return Name.takeError();
    if (*Name != ".drectve")
      continue;
    Expected<StringRef> Data = S.getContents();
    if (!Data)
      return Data.takeError();
    StringRef D = *Data;
    // A UTF-8 BOM may start the section.
    D.consume_front("\xEF\xBB\xBF");
    SmallVector<const char *, 16> Toks;
    cl::TokenizeWindowsCommandLine(D, Saver, Toks);
    for (StringRef T : Toks) {
      if (T.size() < 8 || !(T[0] == '/' || T[0] == '-') ||
          !T.substr(1, 7).equals_insensitive("export:"))
        continue;
      COFFShortExport E;
      if (Error Err = parseExportSpec(T.substr(8), E))
        return Err;
      Exports.push_back(std::move(E));
    }
  }
  return Error::success();
}

static StringRef entryName(const COFFShortExport &E) {
  return E.ExtName.empty() ? StringRef(E.Name) : StringRef(E.ExtName);
}

void libimplib::finalizeExports(std::vector<COFFShortExport> &Exports) {
  StringSet<> Seen;
  std::vector<COFFShortExport> Out;
  for (COFFShortExport &E : Exports)
    if (Seen.insert(entryName(E)).second)
      Out.push_back(std::move(E));
  llvm::stable_sort(Out, [](const COFFShortExport &A,
                            const COFFShortExport &B) {
    return entryName(A) < entryName(B);
  });
  uint16_t Max = 0;
  for (const COFFShortExport &E : Out)
    Max = std::max(Max, E.Ordinal);
  for (COFFShortExport &E : Out)
    if (E.Ordinal == 0)
      E.Ordinal = ++Max;
  Exports = std::move(Out);
}

namespace {
// Little COFF object writer for the one-section exports object.
struct ExpWriter {
  struct Sym {
    std::string Name;
    uint32_t Value;
    int16_t Section; // 1 = .edata, 0 = undefined
    uint8_t Class;
  };
  struct Reloc {
    uint32_t Offset;
    uint32_t Symbol;
  };
  std::vector<uint8_t> Data;
  std::vector<Sym> Syms;
  std::vector<Reloc> Relocs;

  uint32_t addSym(StringRef Name, uint32_t Value, int16_t Section,
                  uint8_t Class) {
    Syms.push_back({Name.str(), Value, Section, Class});
    return Syms.size() - 1;
  }
  void put32(uint32_t Off, uint32_t V) {
    support::endian::write32le(Data.data() + Off, V);
  }
  void put16(uint32_t Off, uint16_t V) {
    support::endian::write16le(Data.data() + Off, V);
  }
  uint32_t addString(StringRef S) {
    uint32_t Off = Data.size();
    Data.insert(Data.end(), S.begin(), S.end());
    Data.push_back(0);
    return Off;
  }
};
} // namespace

Error libimplib::writeExportsObject(StringRef Path, StringRef DllName,
                                    ArrayRef<COFFShortExport> Exports,
                                    COFF::MachineTypes Machine) {
  uint16_t RelType;
  switch (Machine) {
  case COFF::IMAGE_FILE_MACHINE_I386:
    RelType = COFF::IMAGE_REL_I386_DIR32NB;
    break;
  case COFF::IMAGE_FILE_MACHINE_ARMNT:
    RelType = COFF::IMAGE_REL_ARM_ADDR32NB;
    break;
  case COFF::IMAGE_FILE_MACHINE_ARM64:
  case COFF::IMAGE_FILE_MACHINE_ARM64EC:
  case COFF::IMAGE_FILE_MACHINE_ARM64X:
    RelType = COFF::IMAGE_REL_ARM64_ADDR32NB;
    break;
  default:
    RelType = COFF::IMAGE_REL_AMD64_ADDR32NB;
    break;
  }

  uint32_t Base = 0, MaxOrd = 0;
  std::vector<const COFFShortExport *> Named;
  for (const COFFShortExport &E : Exports) {
    Base = Base ? std::min<uint32_t>(Base, E.Ordinal) : E.Ordinal;
    MaxOrd = std::max<uint32_t>(MaxOrd, E.Ordinal);
    if (!E.Noname)
      Named.push_back(&E);
  }
  if (Exports.empty())
    Base = 1;
  uint32_t NumFuncs = Exports.empty() ? 0 : MaxOrd - Base + 1;
  llvm::stable_sort(Named, [](const COFFShortExport *A,
                              const COFFShortExport *B) {
    return entryName(*A) < entryName(*B);
  });

  // Layout as link.exe: directory, address table (rgpv), name pointers
  // (rgszName), ordinals (rgwOrd), DLL name (szName), entry names ($Nxxxxx).
  ExpWriter W;
  const uint32_t EAT = 40, NPT = EAT + 4 * NumFuncs, OT = NPT + 4 * Named.size(),
                 Str = OT + 2 * Named.size();
  W.Data.assign(Str, 0);
  uint32_t SecSym = W.addSym(".edata", 0, 1, COFF::IMAGE_SYM_CLASS_STATIC);
  (void)SecSym;
  uint32_t NameOff = W.addString(DllName);
  uint32_t SzName = W.addSym("szName", NameOff, 1, COFF::IMAGE_SYM_CLASS_STATIC);
  uint32_t Rgpv = W.addSym("rgpv", EAT, 1, COFF::IMAGE_SYM_CLASS_STATIC);
  uint32_t RgszName =
      W.addSym("rgszName", NPT, 1, COFF::IMAGE_SYM_CLASS_STATIC);
  uint32_t RgwOrd = W.addSym("rgwOrd", OT, 1, COFF::IMAGE_SYM_CLASS_STATIC);

  W.put32(4, 0xFFFFFFFF); // TimeDateStamp (deterministic, as link writes)
  W.put32(16, Base);
  W.put32(20, NumFuncs);
  W.put32(24, Named.size());
  W.Relocs.push_back({12, SzName});
  W.Relocs.push_back({28, Rgpv});
  W.Relocs.push_back({32, RgszName});
  W.Relocs.push_back({36, RgwOrd});

  for (uint32_t I = 0; I != Named.size(); ++I) {
    const COFFShortExport *E = Named[I];
    char Label[16];
    snprintf(Label, sizeof(Label), "$N%05u", I + 1);
    uint32_t Off = W.addString(entryName(*E));
    uint32_t L = W.addSym(Label, Off, 1, COFF::IMAGE_SYM_CLASS_STATIC);
    W.Relocs.push_back({NPT + 4 * I, L});
    W.put16(OT + 2 * I, E->Ordinal - Base);
  }
  // Address table: the exported symbols (undefined here), or a forwarder
  // string ("dll.name" internal name).
  for (const COFFShortExport &E : Exports) {
    uint32_t Slot = EAT + 4 * (E.Ordinal - Base);
    if (StringRef(E.Name).contains('.') && !E.ExtName.empty()) {
      uint32_t Off = W.addString(E.Name);
      W.put32(Slot, Off);
      W.Relocs.push_back({Slot, SecSym});
      continue;
    }
    std::string Sym = E.Name;
    if (Machine == COFF::IMAGE_FILE_MACHINE_I386 && !Sym.empty() &&
        Sym[0] != '?' && Sym[0] != '@')
      Sym = "_" + Sym;
    uint32_t S = W.addSym(Sym, 0, 0, COFF::IMAGE_SYM_CLASS_EXTERNAL);
    W.Relocs.push_back({Slot, S});
  }

  // File: header, one section header, raw data, relocations, symbols,
  // string table.
  const uint32_t HdrSize = sizeof(coff_file_header) + sizeof(coff_section);
  uint32_t RawSize = W.Data.size();
  uint32_t RelOff = HdrSize + RawSize;
  uint32_t SymOff = RelOff + W.Relocs.size() * COFF::RelocationSize;
  std::string StrTab(4, '\0');
  std::vector<uint8_t> Out(SymOff + W.Syms.size() * COFF::Symbol16Size, 0);
  using namespace support::endian;
  uint8_t *P = Out.data();
  write16le(P + 0, Machine);
  write16le(P + 2, 1);
  write32le(P + 8, SymOff);
  write32le(P + 12, W.Syms.size());
  uint8_t *SH = P + sizeof(coff_file_header);
  memcpy(SH, ".edata", 6);
  write32le(SH + 16, RawSize);
  write32le(SH + 20, HdrSize);
  write32le(SH + 24, RelOff);
  write16le(SH + 32, W.Relocs.size());
  write32le(SH + 36, COFF::IMAGE_SCN_CNT_INITIALIZED_DATA |
                         COFF::IMAGE_SCN_ALIGN_4BYTES |
                         COFF::IMAGE_SCN_MEM_READ);
  memcpy(P + HdrSize, W.Data.data(), RawSize);
  for (size_t I = 0; I != W.Relocs.size(); ++I) {
    uint8_t *R = P + RelOff + I * COFF::RelocationSize;
    write32le(R + 0, W.Relocs[I].Offset);
    write32le(R + 4, W.Relocs[I].Symbol);
    write16le(R + 8, RelType);
  }
  for (size_t I = 0; I != W.Syms.size(); ++I) {
    uint8_t *S = P + SymOff + I * COFF::Symbol16Size;
    const ExpWriter::Sym &Y = W.Syms[I];
    if (Y.Name.size() <= COFF::NameSize) {
      memcpy(S, Y.Name.data(), Y.Name.size());
    } else {
      write32le(S + 4, StrTab.size());
      StrTab += Y.Name;
      StrTab.push_back('\0');
    }
    write32le(S + 8, Y.Value);
    write16le(S + 12, static_cast<uint16_t>(Y.Section));
    S[16] = Y.Class;
  }
  write32le(reinterpret_cast<uint8_t *>(StrTab.data()), StrTab.size());
  Out.insert(Out.end(), StrTab.begin(), StrTab.end());

  Expected<std::unique_ptr<FileOutputBuffer>> FB =
      FileOutputBuffer::create(Path, Out.size());
  if (!FB)
    return FB.takeError();
  memcpy((*FB)->getBufferStart(), Out.data(), Out.size());
  return (*FB)->commit();
}
