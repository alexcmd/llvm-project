//===-- TextEncoding.cpp - Text encoding conversion class ---------*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file provides utility classes to convert between different character
/// encodings.
///
//===----------------------------------------------------------------------===//

#include "llvm/Support/TextEncoding.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/config.h"
#include "llvm/Support/ConvertEBCDIC.h"
#include "llvm/Support/ConvertUTF.h"
#include <algorithm>
#include <system_error>

#ifdef _WIN32
#include "llvm/Support/Windows/WindowsSupport.h"
#endif

#if HAVE_ICU
#if HAVE_WINDOWS_ICU
#include <icu.h>
#else
#include <unicode/ucnv.h>
#endif
#elif HAVE_ICONV
#include <iconv.h>
#endif

using namespace llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
static void normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<char> &Normalized) {
  bool PrevDigit = false;
  for (auto Ch : CSName) {
    if (isAlnum(Ch)) {
      Ch = toLower(Ch);
      if (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.push_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
static std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  if (Normalized.equals("utf8"))
    return TextEncoding::UTF8;
  if (Normalized.equals("ibm1047"))
    return TextEncoding::IBM1047;
  return std::nullopt;
}

std::optional<unsigned> llvm::getWindowsCodePage(StringRef Name) {
  std::string N = Name.lower();
  StringRef S(N);
  unsigned CP = 0;
  if (S.consume_front(".") || S.consume_front("windows-")) {
    if (S.empty() || S.getAsInteger(10, CP) || CP == 0)
      return std::nullopt;
    return CP;
  }
  if (S.consume_front("iso-8859-")) {
    unsigned Part = 0;
    if (S.getAsInteger(10, Part) || Part == 0 || Part > 16)
      return std::nullopt;
    return 28590 + Part;
  }
  static const struct {
    const char *Name;
    unsigned CP;
  } Names[] = {{"utf-8", 65001},     {"utf8", 65001},
               {"us-ascii", 20127},  {"ascii", 20127},
               {"latin1", 28591},    {"shift_jis", 932},
               {"shift-jis", 932},   {"gb2312", 936},
               {"gbk", 936},         {"big5", 950},
               {"ks_c_5601-1987", 949}, {"euc-kr", 51949},
               {"euc-jp", 20932},    {"koi8-r", 20866},
               {"ibm437", 437},      {"ibm850", 850}};
  for (const auto &E : Names)
    if (S == E.Name)
      return E.CP;
  return std::nullopt;
}

namespace {
// openrt CL-EXEC-CHARSET: the single-byte code pages are built in (the same
// result on every host); the others go through the Windows API on a Windows
// host and are not available elsewhere.
#include "WindowsCodePages.inc"

const uint16_t *singleByteTable(unsigned CP) {
  for (const auto &T : SingleByteTables)
    if (T.CP == CP)
      return T.High;
  return nullptr;
}

void appendUTF8(UTF32 C, SmallVectorImpl<char> &Result) {
  char Buf[UNI_MAX_UTF8_BYTES_PER_CODE_POINT];
  char *P = Buf;
  ConvertCodePointToUTF8(C, P);
  Result.append(Buf, P);
}

// Code pages whose ASCII range is ASCII (most of them); the others (EBCDIC,
// ISO-2022, symbol) are converted character by character.
bool isASCIICompatible(unsigned CP) {
  if (singleByteTable(CP))
    return true;
  switch (CP) {
  case 932: case 936: case 949: case 950: case 1361: case 20127:
  case 20932: case 20936: case 20949: case 51932: case 51936: case 51949:
  case 54936:
    return true;
  default:
    return false;
  }
}

// WideCharToMultiByte takes no flags and no default character for these
// (documented); unrepresentable characters are then found by a round trip.
bool windowsNeedsNoFlags(unsigned CP) {
  return CP == 42 || (CP >= 50220 && CP <= 50229) || CP == 52936 ||
         CP == 54936 || (CP >= 57002 && CP <= 57011) || CP == 65000;
}

class TextEncodingConverterWindowsCP final
    : public details::TextEncodingConverterImplBase {
  unsigned CodePage;
  bool FromUTF8;
  const uint16_t *Table;

public:
  TextEncodingConverterWindowsCP(unsigned CodePage, bool FromUTF8)
      : CodePage(CodePage), FromUTF8(FromUTF8),
        Table(singleByteTable(CodePage)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<char> &Result) override {
    Result.clear();
    // Plain ASCII is the same in an ASCII-compatible code page.
    if (isASCIICompatible(CodePage) &&
        llvm::all_of(Source, [](char C) { return (unsigned char)C < 0x80; })) {
      Result.append(Source.begin(), Source.end());
      return std::error_code();
    }
    if (Table)
      return FromUTF8 ? toTable(Source, Result) : fromTable(Source, Result);
#ifdef _WIN32
    return FromUTF8 ? toWindows(Source, Result) : fromWindows(Source, Result);
#else
    return std::make_error_code(std::errc::invalid_argument);
#endif
  }

  void reset() override {}

private:
  int encodeTable(UTF32 C) const {
    if (C < 0x80)
      return (int)C;
    for (unsigned I = 0; I != 128; ++I)
      if (Table[I] == C)
        return (int)(0x80 + I);
    return -1;
  }

  std::error_code toTable(StringRef Source, SmallVectorImpl<char> &Result) {
    bool Substituted = false;
    const UTF8 *P = reinterpret_cast<const UTF8 *>(Source.begin());
    const UTF8 *E = reinterpret_cast<const UTF8 *>(Source.end());
    while (P < E) {
      UTF32 C = 0;
      if (convertUTF8Sequence(&P, E, &C, strictConversion) != conversionOK)
        return std::make_error_code(std::errc::illegal_byte_sequence);
      int B = encodeTable(C);
      if (B < 0) {
        B = '?';
        Substituted = true;
      }
      Result.push_back((char)B);
    }
    return Substituted ? std::make_error_code(std::errc::result_out_of_range)
                       : std::error_code();
  }

  std::error_code fromTable(StringRef Source, SmallVectorImpl<char> &Result) {
    for (unsigned char B : Source)
      appendUTF8(B < 0x80 ? B : Table[B - 0x80], Result);
    return std::error_code();
  }

#ifdef _WIN32
  std::error_code toWindows(StringRef Source, SmallVectorImpl<char> &Result) {
    if (Source.empty())
      return std::error_code();
    int WN = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Source.data(),
                                   (int)Source.size(), nullptr, 0);
    if (WN <= 0)
      return std::make_error_code(std::errc::illegal_byte_sequence);
    SmallVector<wchar_t, 64> W(WN);
    ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Source.data(),
                          (int)Source.size(), W.data(), WN);
    bool NoFlags = windowsNeedsNoFlags(CodePage);
    BOOL UsedDefault = FALSE;
    DWORD Flags = NoFlags ? 0 : WC_NO_BEST_FIT_CHARS;
    const char *Default = NoFlags ? nullptr : "?";
    BOOL *UsedPtr = NoFlags ? nullptr : &UsedDefault;
    int N = ::WideCharToMultiByte(CodePage, Flags, W.data(), WN, nullptr, 0,
                                  Default, UsedPtr);
    if (N <= 0)
      return std::make_error_code(std::errc::invalid_argument);
    Result.resize(N);
    UsedDefault = FALSE;
    ::WideCharToMultiByte(CodePage, Flags, W.data(), WN, Result.data(), N,
                          Default, UsedPtr);
    if (NoFlags) {
      // No default-character report for these: compare a round trip.
      int BN = ::MultiByteToWideChar(CodePage, 0, Result.data(), N, nullptr, 0);
      SmallVector<wchar_t, 64> Back(BN > 0 ? BN : 0);
      if (BN > 0)
        ::MultiByteToWideChar(CodePage, 0, Result.data(), N, Back.data(), BN);
      UsedDefault = BN != WN || !std::equal(W.begin(), W.end(), Back.begin());
    }
    // ISO-2022-JP (50220-50222): the Windows API writes a character of JIS X
    // 0212 (ESC $ ( D ... ESC ( B), which cl does not use: C4566 and the
    // character is left out (observed: "\u00E9" gives an empty literal).
    if (CodePage >= 50220 && CodePage <= 50222) {
      static const char JIS0212[] = "\x1b$(D", ASCIIBack[] = "\x1b(B";
      std::string Kept;
      StringRef R(Result.data(), Result.size());
      while (!R.empty()) {
        size_t I = R.find(JIS0212);
        Kept += R.substr(0, I).str();
        if (I == StringRef::npos)
          break;
        UsedDefault = TRUE;
        R = R.drop_front(I + 4);
        size_t J = R.find(ASCIIBack);
        R = J == StringRef::npos ? StringRef() : R.drop_front(J + 3);
      }
      Result.assign(Kept.begin(), Kept.end());
    }
    return UsedDefault ? std::make_error_code(std::errc::result_out_of_range)
                       : std::error_code();
  }

  std::error_code fromWindows(StringRef Source,
                              SmallVectorImpl<char> &Result) {
    if (Source.empty())
      return std::error_code();
    int WN = ::MultiByteToWideChar(CodePage, 0, Source.data(),
                                   (int)Source.size(), nullptr, 0);
    if (WN <= 0)
      return std::make_error_code(std::errc::illegal_byte_sequence);
    SmallVector<wchar_t, 64> W(WN);
    ::MultiByteToWideChar(CodePage, 0, Source.data(), (int)Source.size(),
                          W.data(), WN);
    int N = ::WideCharToMultiByte(CP_UTF8, 0, W.data(), WN, nullptr, 0,
                                  nullptr, nullptr);
    Result.resize(N);
    ::WideCharToMultiByte(CP_UTF8, 0, W.data(), WN, Result.data(), N, nullptr,
                          nullptr);
    return std::error_code();
  }
#endif
};

bool isUsableWindowsCodePage(unsigned CP) {
  if (CP == 65001)
    return false;
  if (singleByteTable(CP))
    return true;
#ifdef _WIN32
  CPINFO Info;
  if (!::GetCPInfo(CP, &Info))
    return false;
  // A test conversion, so that a page the API cannot write fails here.
  TextEncodingConverterWindowsCP Test(CP, /*FromUTF8=*/true);
  SmallString<8> Out;
  std::error_code EC = Test.convert("a\xC3\xA9", Out);
  return !EC || EC == std::errc::result_out_of_range;
#else
  return false;
#endif
}
} // namespace

[[maybe_unused]] static void HandleOverflow(size_t &Capacity, char *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<char> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<char *>(Result.data());
  OutputLength = Capacity;
}

namespace {
enum ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
class TextEncodingConverterTable final
    : public details::TextEncodingConverterImplBase {
  const ConversionType ConvType;

public:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<char> &Result) override;

  void reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<char> &Result) {
  switch (ConvType) {
  case IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    return std::error_code();
  case UTF8ToIBM1047:
    return ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  return std::error_code();
}

#if HAVE_ICU
struct UConverterDeleter {
  void operator()(UConverter *Converter) const {
    if (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

class TextEncodingConverterICU final
    : public details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

public:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<char> &Result) override;

  void reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<char> &Result) {
  // Setup the input in case it has no backing data.
  size_t InputLength = Source.size();
  const char *In = InputLength ? const_cast<char *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  char *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, NULL, NULL,
                        NULL, &EC);
  assert(U_SUCCESS(EC));

  do {
    EC = U_ZERO_ERROR;
    const char *Input = In;

    Output = static_cast<char *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &Input,
                   In + InputLength, /*pivotStart=*/NULL,
                   /*pivotSource=*/NULL, /*pivotTarget=*/NULL,
                   /*pivotLimit=*/NULL, /*reset=*/true,
                   /*flush=*/true, &EC);
    if (U_FAILURE(EC)) {
      if (EC == U_BUFFER_OVERFLOW_ERROR) {
        if (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          continue;
        } else {
          Result.resize(Output - Result.data());
          return std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      return std::error_code(EILSEQ, std::generic_category());
    }
    break;
  } while (true);

  Result.resize(Output - Result.data());
  return std::error_code();
}

void TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#elif HAVE_ICONV
class TextEncodingConverterIconv final
    : public details::TextEncodingConverterImplBase {
  class UniqueIconvT {
    iconv_t ConvDesc;

  public:
    operator iconv_t() const { return ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      if (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      if (&Other != this) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      return *this;
    }
  };
  UniqueIconvT ConvDesc;

public:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<char> &Result) override;

  void reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<char> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  char *Output = static_cast<char *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      this](size_t Ret) {
    if (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      if (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        return std::error_code();
      } else {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        return std::error_code(errno, std::generic_category());
      }
    } else {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      return std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  do {
    size_t InputLength = Source.size();
    char *Input = const_cast<char *>(Source.data());
    Ret = iconv(ConvDesc, &Input, &InputLength, &Output, &OutputLength);
    if (Ret != 0) {
      if (auto EC = HandleError(Ret))
        return EC;
      continue;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    if (Ret != 0) {
      if (auto EC = HandleError(Ret))
        return EC;
      continue;
    }
    break;
  } while (true);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  return std::error_code();
}

inline void TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  if (CPFrom == CPTo)
    return std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  if (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  else if (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  else
    return std::make_error_code(std::errc::invalid_argument);

  return TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  // openrt CL-EXEC-CHARSET: UTF-8 to or from a Windows code page.
  if (FromEncoding == TextEncoding::UTF8 || ToEncoding == TextEncoding::UTF8) {
    std::optional<unsigned> CP =
        getWindowsCodePage(FromEncoding == TextEncoding::UTF8 ? To : From);
    if (CP && *CP != 65001 && isUsableWindowsCodePage(*CP))
      return TextEncodingConverter(
          std::make_unique<TextEncodingConverterWindowsCP>(
              *CP, FromEncoding == TextEncoding::UTF8));
  }
  if (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    if (Converter)
      return Converter;
  }
#if HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  if (U_FAILURE(EC))
    return std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  if (U_FAILURE(EC))
    return std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  return TextEncodingConverter(std::move(Converter));
#elif HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  if (ConvDesc == (iconv_t)-1)
    return std::make_error_code(std::errc::invalid_argument);
  return TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#else
  return std::make_error_code(std::errc::invalid_argument);
#endif
}
