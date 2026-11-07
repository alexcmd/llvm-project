# TODO: Add -maltivec when ARCH is PowerPC.

set(LLVM_LINK_COMPONENTS
  Support
  TargetParser
  )

add_clang_library(clangLex
  DependencyDirectivesScanner.cpp
  HeaderMap.cpp
  HeaderSearch.cpp
  InitHeaderSearch.cpp
  Lexer.cpp
  LexHLSLRootSignature.cpp
  LiteralSupport.cpp
  MacroArgs.cpp
  MacroInfo.cpp
  ModuleMap.cpp
  ModuleMapFile.cpp
  PPCaching.cpp
  PPCallbacks.cpp
  PPConditionalDirectiveRecord.cpp
  PPDirectives.cpp
  PPExpressions.cpp
  PPLexerChange.cpp
  PPMacroExpansion.cpp
  Pragma.cpp
  PreprocessingRecord.cpp
  Preprocessor.cpp
  PreprocessorLexer.cpp
  ScratchBuffer.cpp
  TextEncoding.cpp
  TokenConcatenation.cpp
  TokenLexer.cpp
  TypeLibImport.cpp

  LINK_LIBS
  clangBasic
  )

# #import of a type library reads it through the COM automation API.
if(WIN32)
  target_link_libraries(clangLex PRIVATE ole32 oleaut32 advapi32)
endif()
