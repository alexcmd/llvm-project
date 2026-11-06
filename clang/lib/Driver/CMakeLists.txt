set(LLVM_LINK_COMPONENTS
  BinaryFormat
  MC
  Object
  Option
  ProfileData
  Support
  TargetParser
  WindowsDriver
  )

if(WIN32)
  # MSVCToolChain.cpp uses version.dll.
  set(system_libs version)
endif()

if(CLANG_USE_XCSELECT)
  set(system_libs xcselect)
endif()

if(LLVM_PTHREAD_LIB)
  list(APPEND system_libs ${LLVM_PTHREAD_LIB})
endif()

# OPENRT-PREDEF: clang-cl predefines __OPENRT__ as major*100+minor of the
# openrt release this compiler belongs to (0.1 -> 1, 1.2 -> 102).
set(CLANG_OPENRT_RELEASE "0.1" CACHE STRING
  "openrt release (major.minor) behind the clang-cl __OPENRT__ predefine")
if(NOT CLANG_OPENRT_RELEASE MATCHES "^([0-9]+)\\.([0-9]+)$")
  message(FATAL_ERROR "CLANG_OPENRT_RELEASE must be <major>.<minor>")
endif()
math(EXPR CLANG_OPENRT_RELEASE_VALUE
  "${CMAKE_MATCH_1} * 100 + ${CMAKE_MATCH_2}")
set_property(SOURCE ToolChains/Clang.cpp APPEND PROPERTY COMPILE_DEFINITIONS
  CLANG_OPENRT_RELEASE_VALUE=${CLANG_OPENRT_RELEASE_VALUE})

add_clang_library(clangDriver
  Action.cpp
  Compilation.cpp
  CreateASTUnitFromArgs.cpp
  CreateInvocationFromArgs.cpp
  Distro.cpp
  Driver.cpp
  Job.cpp
  ModulesDriver.cpp
  Multilib.cpp
  MultilibBuilder.cpp
  OffloadBundler.cpp
  Phases.cpp
  SanitizerArgs.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains/Arch/AArch64.cpp
  ToolChains/Arch/AMDGPU.cpp
  ToolChains/Arch/ARM.cpp
  ToolChains/Arch/CSKY.cpp
  ToolChains/Arch/LoongArch.cpp
  ToolChains/Arch/M68k.cpp
  ToolChains/Arch/Mips.cpp
  ToolChains/Arch/PPC.cpp
  ToolChains/Arch/RISCV.cpp
  ToolChains/Arch/Sparc.cpp
  ToolChains/Arch/SystemZ.cpp
  ToolChains/Arch/VE.cpp
  ToolChains/Arch/X86.cpp
  ToolChains/AIX.cpp
  ToolChains/AMDGPU.cpp
  ToolChains/AVR.cpp
  ToolChains/BareMetal.cpp
  ToolChains/Clang.cpp
  ToolChains/CommonArgs.cpp
  ToolChains/CrossWindows.cpp
  ToolChains/CSKYToolChain.cpp
  ToolChains/Cuda.cpp
  ToolChains/Cygwin.cpp
  ToolChains/Darwin.cpp
  ToolChains/DragonFly.cpp
  ToolChains/Flang.cpp
  ToolChains/FreeBSD.cpp
  ToolChains/Fuchsia.cpp
  ToolChains/Gnu.cpp
  ToolChains/Haiku.cpp
  ToolChains/HIPUtility.cpp
  ToolChains/HIPAMD.cpp
  ToolChains/HIPSPV.cpp
  ToolChains/Hexagon.cpp
  ToolChains/HLSL.cpp
  ToolChains/Hurd.cpp
  ToolChains/LFILinux.cpp
  ToolChains/Linux.cpp
  ToolChains/Managarm.cpp
  ToolChains/MipsLinux.cpp
  ToolChains/MinGW.cpp
  ToolChains/MSP430.cpp
  ToolChains/MSVC.cpp
  ToolChains/NetBSD.cpp
  ToolChains/OHOS.cpp
  ToolChains/OpenBSD.cpp
  ToolChains/PS4CPU.cpp
  ToolChains/Serenity.cpp
  ToolChains/Solaris.cpp
  ToolChains/SPIRV.cpp
  ToolChains/SPIRVOpenMP.cpp
  ToolChains/SYCL.cpp
  ToolChains/TCE.cpp
  ToolChains/UEFI.cpp
  ToolChains/VEToolchain.cpp
  ToolChains/WebAssembly.cpp
  ToolChains/XCore.cpp
  ToolChains/PPCLinux.cpp
  ToolChains/PPCFreeBSD.cpp
  ToolChains/InterfaceStubs.cpp
  ToolChains/ZOS.cpp
  Types.cpp
  XRayArgs.cpp

  DEPENDS
  ClangDriverOptions
  # These generated headers are included transitively.
  target_parser_gen

  LINK_LIBS
  clangBasic
  clangDependencyScanning
  clangFrontend
  clangScalableStaticAnalysisAnalyses
  clangScalableStaticAnalysisCore
  clangScalableStaticAnalysisFrontend
  clangScalableStaticAnalysisSourceTransformation
  clangSerialization
  clangLex
  clangOptions
  ${system_libs}
  )
