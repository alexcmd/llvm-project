// Regression test for the MXCSR rounding-mode miscompile: an optimized direct
// call whose result depends on MXCSR-controlled rounding (immediate bit 0b100,
// _MM_FROUND_CUR_DIRECTION) used to be CSE'd across an intervening
// _mm_setcsr / llvm.x86.sse.ldmxcsr, so the second rounding reused the first
// result. The llvm.x86.sse41.round.* / llvm.x86.avx.round.p*_256 intrinsics
// now model a read of the floating-point environment
// (memory(inaccessiblemem: read)); clang keeps fixed-direction call sites
// pure via a memory(none) call-site attribute.

// The -O2 run is the fails-before part: before the fix it contained a single
// round.ps/round.ss call per function below (second rounding reused),
// regardless of the intervening llvm.x86.sse.ldmxcsr calls.

// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -O0 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -O2 %s -emit-llvm -o - | FileCheck %s --check-prefix=OPT
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -ffp-exception-behavior=strict -frounding-math -O2 %s -emit-llvm -o - | FileCheck %s --check-prefix=OPT

// The test stays header-free: declare the vector type the builtins operate on
// (matches the xmmintrin.h definition).
typedef float __v4sf __attribute__((__vector_size__(16)));

// Runtime differential for the standalone repro (direct-mxcsr.cpp). Measured
// 2026-11-11 (D:\Build\ort-p34\fails-before-20261111\README.md): the Nov-10
// compiler candidate returned 2 before the fix; the shipped p33r5 build
// outlines the two identical rounding bodies into one helper called twice, so
// both shapes stay correct (exit 0) with and without the fix, and cl 19.50
// itself returns 2 (it hoists all three ldmxcsr above both roundps). The lit
// check above, not this differential, is the pre/post discriminator.

// A current-direction rounding must not be reused across an MXCSR change:
// both roundings (and both ldmxcsr writes) must survive the optimizer.
extern "C" __v4sf round_cur_across_setcsr(__v4sf x, unsigned a, unsigned b) {
  __builtin_ia32_ldmxcsr(a);
  __v4sf r1 = __builtin_ia32_roundps(x, 4);
  __builtin_ia32_ldmxcsr(b);
  __v4sf r2 = __builtin_ia32_roundps(x, 4);
  return r1 + r2;
}

// OPT-LABEL: define {{.*}}@round_cur_across_setcsr
// OPT: call {{.*}}@llvm.x86.sse.ldmxcsr({{.*}})
// OPT: call {{.*}}@llvm.x86.sse41.round.ps({{.*}}i32 4)
// OPT: call {{.*}}@llvm.x86.sse.ldmxcsr({{.*}})
// OPT: call {{.*}}@llvm.x86.sse41.round.ps({{.*}}i32 4)

extern "C" __v4sf roundss_cur_across_setcsr(__v4sf x, __v4sf y, unsigned a,
                                            unsigned b) {
  __builtin_ia32_ldmxcsr(a);
  __v4sf r1 = __builtin_ia32_roundss(x, y, 4);
  __builtin_ia32_ldmxcsr(b);
  __v4sf r2 = __builtin_ia32_roundss(x, y, 4);
  return r1 + r2;
}

// OPT-LABEL: define {{.*}}@roundss_cur_across_setcsr
// OPT: call {{.*}}@llvm.x86.sse.ldmxcsr({{.*}})
// OPT: call {{.*}}@llvm.x86.sse41.round.ss({{.*}}i32 4)
// OPT: call {{.*}}@llvm.x86.sse.ldmxcsr({{.*}})
// OPT: call {{.*}}@llvm.x86.sse41.round.ss({{.*}}i32 4)

// Without an MXCSR change in between, identical current-direction calls must
// still be CSE'd into one rounding (no codegen change for programs that do
// not touch the FP environment).
extern "C" __v4sf round_cur_cse(__v4sf x) {
  __v4sf r1 = __builtin_ia32_roundps(x, 4);
  __v4sf r2 = __builtin_ia32_roundps(x, 4);
  return r1 + r2;
}

// OPT-LABEL: define {{.*}}@round_cur_cse
// OPT: call {{.*}}@llvm.x86.sse41.round.ps({{.*}}i32 4)
// OPT-NOT: call {{.*}}@llvm.x86.sse41.round.ps

// A fixed-direction rounding does not observe MXCSR, so it must stay pure
// (memory(none) call site) and must still be CSE'd even across setcsr.
extern "C" __v4sf round_fixed_across_setcsr(__v4sf x, unsigned a, unsigned b) {
  __builtin_ia32_ldmxcsr(a);
  __v4sf r1 = __builtin_ia32_roundps(x, 1);
  __builtin_ia32_ldmxcsr(b);
  __v4sf r2 = __builtin_ia32_roundps(x, 1);
  return r1 + r2;
}

// OPT-LABEL: define {{.*}}@round_fixed_across_setcsr
// OPT: call {{.*}}@llvm.x86.sse41.round.ps({{.*}}i32 1) #[[MEMNONE:[0-9]+]]
// OPT-NOT: call {{.*}}@llvm.x86.sse41.round.ps
// OPT: attributes #[[MEMNONE]] = {{.*}}memory(none)

// -O0 structural checks: both call sites are emitted, the fixed-direction
// call carries the memory(none) call-site attribute group, and the rounding
// intrinsic declares a read of the FP environment.

// CHECK-LABEL: define {{.*}}@round_cur_across_setcsr
// CHECK: call {{.*}}@llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 4)
// CHECK: call {{.*}}@llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 4)

// CHECK-LABEL: define {{.*}}@round_fixed_across_setcsr
// CHECK: call {{.*}}@llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 1) #[[FIXEDATTR:[0-9]+]]
// CHECK: attributes #[[FIXEDATTR]] = { memory(none) }
// CHECK: declare <4 x float> @llvm.x86.sse41.round.ps(<4 x float>, i32 immarg) #[[FPENV:[0-9]+]]
// CHECK: attributes #[[FPENV]] = {{.*}}memory(inaccessiblemem: read)
