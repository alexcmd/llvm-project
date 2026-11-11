// Regression test for the MXCSR rounding-mode miscompile: an optimized direct
// call whose result depends on MXCSR-controlled rounding (immediate bit 0b100,
// _MM_FROUND_CUR_DIRECTION) used to be CSE'd across an intervening
// _mm_setcsr / llvm.x86.sse.ldmxcsr, so the second rounding reused the first
// result. The llvm.x86.sse41.round.* / llvm.x86.avx.round.p*_256 intrinsics
// now model a read of the floating-point environment
// (memory(inaccessiblemem: read)); clang keeps fixed-direction call sites
// pure via a memory(none) call-site attribute.

// The -O2 run is the fails-before part for the CSE bug: before the fix it
// contained a single round.ps/round.ss call per function below (second
// rounding reused), regardless of the intervening llvm.x86.sse.ldmxcsr calls.

// The two -S runs are the fails-before part for the backend regression the
// .td change caused: a tree with the intrinsic property change but without
// the X86 chain lowering (X86IntrinsicsInfo.h IntrinsicsWithChain and the
// ROUNDP/ROUNDS cases in LowerINTRINSIC_W_CHAIN) crashed the backend with
// "Cannot select" on every rounding form, because SelectionDAGBuilder builds
// the call as INTRINSIC_W_CHAIN from the callee declaration's memory effects
// (the call site's are deliberately ignored there).

// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -O0 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -O2 %s -emit-llvm -o - | FileCheck %s --check-prefix=OPT
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -ffp-exception-behavior=strict -frounding-math -O2 %s -emit-llvm -o - | FileCheck %s --check-prefix=OPT
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -O0 %s -S -o - | FileCheck %s --check-prefix=CODEGEN
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -target-feature +sse4.1 -ffreestanding -O2 %s -S -o - | FileCheck %s --check-prefix=OPTCODEGEN

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
// intrinsic declares a read of the FP environment. At -O0 clang emits each
// used intrinsic's declaration after the first function body that uses it,
// so the declare check follows the round_cur body checks; the two attribute
// groups come last and are checked as a DAG.

// CHECK-LABEL: define {{.*}}@round_cur_across_setcsr
// CHECK: call {{.*}}@llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 4)
// CHECK: call {{.*}}@llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 4)
// CHECK: declare <4 x float> @llvm.x86.sse41.round.ps(<4 x float>, i32 immarg) #[[FPENV:[0-9]+]]

// CHECK-LABEL: define {{.*}}@round_fixed_across_setcsr
// CHECK: call {{.*}}@llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 1) #[[FIXEDATTR:[0-9]+]]
// CHECK-DAG: attributes #[[FIXEDATTR]] = { memory(none) }
// CHECK-DAG: attributes #[[FPENV]] = {{.*}}memory(inaccessiblemem: read)

// The scalar-double form reaches the chained lowering through the ROUNDS
// case (roundss above goes through it as well).
typedef double __v2df __attribute__((__vector_size__(16)));

extern "C" __v2df roundsd_cur_across_setcsr(__v2df x, __v2df y, unsigned a,
                                            unsigned b) {
  __builtin_ia32_ldmxcsr(a);
  __v2df r1 = __builtin_ia32_roundsd(x, y, 4);
  __builtin_ia32_ldmxcsr(b);
  __v2df r2 = __builtin_ia32_roundsd(x, y, 4);
  return r1 + r2;
}

// The AVX 256-bit forms are the shapes the p34 candidate gate crashed on
// (llvm.x86.avx.round.pd.256 in clang/test/CodeGen/X86/avx-builtins-
// constrained.c). Kept header-free with a function-level target attribute.
typedef float __v8sf __attribute__((__vector_size__(32)));
typedef double __v4df __attribute__((__vector_size__(32)));

extern "C" __attribute__((target("avx"))) __v8sf
round256ps_cur_across_setcsr(__v8sf x, unsigned a, unsigned b) {
  __builtin_ia32_ldmxcsr(a);
  __v8sf r1 = __builtin_ia32_roundps256(x, 4);
  __builtin_ia32_ldmxcsr(b);
  __v8sf r2 = __builtin_ia32_roundps256(x, 4);
  return r1 + r2;
}

extern "C" __attribute__((target("avx"))) __v4df
round256pd_cur_across_setcsr(__v4df x, unsigned a, unsigned b) {
  __builtin_ia32_ldmxcsr(a);
  __v4df r1 = __builtin_ia32_roundpd256(x, 4);
  __builtin_ia32_ldmxcsr(b);
  __v4df r2 = __builtin_ia32_roundpd256(x, 4);
  return r1 + r2;
}

extern "C" __attribute__((target("avx"))) __v8sf
round256ps_fixed_across_setcsr(__v8sf x, unsigned a, unsigned b) {
  __builtin_ia32_ldmxcsr(a);
  __v8sf r1 = __builtin_ia32_roundps256(x, 1);
  __builtin_ia32_ldmxcsr(b);
  __v8sf r2 = __builtin_ia32_roundps256(x, 1);
  return r1 + r2;
}

// -O0 codegen: every form must select (these runs fatal "Cannot select"
// before the chain lowering exists). Instruction order within a function is
// source order at -O0.

// CODEGEN-LABEL: {{^}}round_cur_across_setcsr:
// CODEGEN: ldmxcsr
// CODEGEN: roundps
// CODEGEN: ldmxcsr
// CODEGEN: roundps

// CODEGEN-LABEL: {{^}}roundss_cur_across_setcsr:
// CODEGEN: ldmxcsr
// CODEGEN-COUNT-2: roundss

// CODEGEN-LABEL: {{^}}round_cur_cse:
// CODEGEN-COUNT-2: roundps

// CODEGEN-LABEL: {{^}}round_fixed_across_setcsr:
// CODEGEN: roundps
// CODEGEN: ldmxcsr
// CODEGEN: roundps

// CODEGEN-LABEL: {{^}}roundsd_cur_across_setcsr:
// CODEGEN: ldmxcsr
// CODEGEN-COUNT-2: roundsd

// CODEGEN-LABEL: {{^}}round256ps_cur_across_setcsr:
// CODEGEN: vroundps
// CODEGEN: ldmxcsr
// CODEGEN: vroundps

// CODEGEN-LABEL: {{^}}round256pd_cur_across_setcsr:
// CODEGEN: vroundpd
// CODEGEN: ldmxcsr
// CODEGEN: vroundpd

// CODEGEN-LABEL: {{^}}round256ps_fixed_across_setcsr:
// CODEGEN-COUNT-2: vroundps

// -O2 codegen: the chain keeps both roundings and both MXCSR writes alive
// and in order (a round MI also reads the MXCSR pseudo-register - LDMXCSR
// writes it - so no scheduler may move one across an ldmxcsr), while the
// fixed-form calls and the no-change current-form calls still CSE down to a
// single instruction.

// OPTCODEGEN-LABEL: {{^}}round_cur_across_setcsr:
// OPTCODEGEN: ldmxcsr
// OPTCODEGEN: roundps
// OPTCODEGEN: ldmxcsr
// OPTCODEGEN: roundps

// OPTCODEGEN-LABEL: {{^}}roundss_cur_across_setcsr:
// OPTCODEGEN-COUNT-2: roundss

// OPTCODEGEN-LABEL: {{^}}round_cur_cse:
// OPTCODEGEN: roundps
// OPTCODEGEN-NOT: roundps

// OPTCODEGEN-LABEL: {{^}}round_fixed_across_setcsr:
// OPTCODEGEN: roundps
// OPTCODEGEN-NOT: roundps

// OPTCODEGEN-LABEL: {{^}}roundsd_cur_across_setcsr:
// OPTCODEGEN-COUNT-2: roundsd

// OPTCODEGEN-LABEL: {{^}}round256ps_cur_across_setcsr:
// OPTCODEGEN-COUNT-2: vroundps

// OPTCODEGEN-LABEL: {{^}}round256pd_cur_across_setcsr:
// OPTCODEGEN-COUNT-2: vroundpd

// OPTCODEGEN-LABEL: {{^}}round256ps_fixed_across_setcsr:
// OPTCODEGEN: vroundps
// OPTCODEGEN-NOT: vroundps
