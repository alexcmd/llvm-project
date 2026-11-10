// REQUIRES: x86-registered-target
// RUN: %clang_cl /c /O2 /arch:AVX2 /fp:fast /clang:-S /clang:-o /clang:- %s | FileCheck %s --check-prefixes=ASM,FAST
// RUN: %clang_cl /c /O2 /arch:AVX2 /fp:precise /clang:-S /clang:-o /clang:- %s | FileCheck %s --check-prefixes=ASM,PRECISE
// RUN: %clang_cl /c /O2 /arch:AVX2 /fp:strict /clang:-S /clang:-o /clang:- %s | FileCheck %s --check-prefixes=ASM,STRICT
// RUN: %clang_cl /c /O2 /arch:AVX2 /clang:-S /clang:-o /clang:- %s | FileCheck %s --check-prefixes=ASM,PRECISE

// ASM-LABEL: initial:
// FAST: vfmadd
// PRECISE: vmulss
// PRECISE: vaddss
// STRICT: vmulss
// STRICT: vaddss
extern "C" float initial(float a, float b, float c) { return a * b + c; }

#pragma float_control(precise, on, push)
#pragma fp_contract(off)
// ASM-LABEL: off:
// ASM-NOT: vfmadd
// ASM: vmulss
// ASM: vaddss
// ASM-NOT: vfmadd
extern "C" float off(float a, float b, float c) { return a * b + c; }

#pragma fp_contract(on)
// ASM-LABEL: on:
// FAST: vfmadd
// PRECISE: vfmadd
// STRICT: vmulss
// STRICT: vaddss
extern "C" float on(float a, float b, float c) { return a * b + c; }
// ASM-LABEL: on_split:
// FAST: vfmadd
// PRECISE: vfmadd
// STRICT: vmulss
// STRICT: vaddss
extern "C" float on_split(float a, float b, float c) {
  float x = a * b;
  return x + c;
}

#pragma fp_contract(off)
#pragma float_control(pop)
// ASM-LABEL: restored:
// FAST: vfmadd
// PRECISE: vmulss
// PRECISE: vaddss
// STRICT: vmulss
// STRICT: vaddss
extern "C" float restored(float a, float b, float c) { return a * b + c; }

namespace N {
#pragma fp_contract(off)
}
// ASM-LABEL: after_namespace:
// ASM-NOT: vfmadd
// ASM: vmulss
// ASM: vaddss
// ASM-NOT: vfmadd
extern "C" float after_namespace(float a, float b, float c) { return a * b + c; }

// ASM-LABEL: ignored_local:
// ASM-NOT: vfmadd
// ASM: vmulss
// ASM: vaddss
// ASM-NOT: vfmadd
extern "C" float ignored_local(float a, float b, float c) {
  float x = a * b;
#pragma fp_contract(on)
  return x + c;
}

#pragma fp_contract(on)
#pragma float_control(except, off)
#pragma fenv_access(off)
// ASM-LABEL: after_strict:
// ASM: vfmadd
extern "C" float after_strict(float a, float b, float c) { return a * b + c; }

#pragma float_control(precise, on)
// ASM-LABEL: precise_resets:
// ASM-NOT: vfmadd
// ASM: vmulss
// ASM: vaddss
// ASM-NOT: vfmadd
extern "C" float precise_resets(float a, float b, float c) { return a * b + c; }

#pragma float_control(precise, off)
// ASM-LABEL: fast_preserves_off:
// ASM-NOT: vfmadd
// ASM: vmulss
// ASM: vaddss
// ASM-NOT: vfmadd
extern "C" float fast_preserves_off(float a, float b, float c) { return a * b + c; }

#pragma fp_contract(on)
#pragma float_control(precise, off)
// ASM-LABEL: fast_preserves_on:
// ASM: vfmadd
extern "C" float fast_preserves_on(float a, float b, float c) { return a * b + c; }
