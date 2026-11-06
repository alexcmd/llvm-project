// MS-MACRO-INTRINSICS-GATING: macro-form x86 intrinsics whose builtin takes
// an immediate (SSE4.1, SSSE3, AVX, AVX2, F16C, AES, PCLMUL, SHA) are template
// wrappers in MS-compat C++ (immediate as template argument, the intrinsic's
// target features on the wrapper). A caller without those features compiles,
// as with cl, and calls the wrapper through the feature thunk
// (MS-INTRIN-ABI-THUNK); a caller with the features inlines as before.

// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=x86_64-pc-windows-msvc \
// RUN:   -fms-compatibility -fms-compatibility-version=19 -fms-extensions \
// RUN:   -emit-llvm -disable-llvm-passes -o - -Wall -Werror | FileCheck %s --check-prefix=IR
// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=x86_64-pc-windows-msvc \
// RUN:   -fms-compatibility -fms-compatibility-version=19 -fms-extensions \
// RUN:   -O2 -S -o - | FileCheck %s --check-prefix=ASM
// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=i686-pc-windows-msvc \
// RUN:   -fms-compatibility -fms-compatibility-version=19 -fms-extensions \
// RUN:   -O2 -S -o - -DNO64 | FileCheck %s --check-prefix=X86
// Non-MS keeps the plain macros.
// RUN: %clang_cc1 -x c++ -ffreestanding %s -triple=x86_64-unknown-linux-gnu \
// RUN:   -target-feature +avx2 -target-feature +f16c -target-feature +aes \
// RUN:   -target-feature +pclmul -target-feature +sha -emit-llvm -o - | FileCheck %s --check-prefix=NOMS

#include <immintrin.h>

// IR-LABEL: define {{.*}}@"?ext@@YAHPEBH@Z"
// IR: call {{.*}}@"??$__ms__mm256_extract_epi32@$02@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm_blend_epi16@$0KF@@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm256_i32gather_epi32@$03@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm_insert_ps@$0EN@@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm_cvtps_ph@$0A@@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm_aeskeygenassist_si128@$0BL@@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm_clmulepi64_si128@$0BB@@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm_sha1rnds4_epu32@$01@@{{.*}}.openrt.xfeat"
// IR: call {{.*}}@"??$__ms__mm_alignr_epi8@$04@@{{.*}}.openrt.xfeat"
// IR: ret i32
// NOMS-LABEL: define {{.*}}@_Z3extPKi
// NOMS-NOT: __ms_
// NOMS: ret i32
#ifdef __x86_64__
int ext(const int *d) {
  __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(d));
  int r = _mm256_extract_epi32(v, 3);
  __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i *>(d));
  __m128i b = _mm_blend_epi16(a, _mm_setzero_si128(), 0xA5);
  __m256i g = _mm256_i32gather_epi32(d, _mm256_setzero_si256(), 4);
  __m128 f = _mm_insert_ps(_mm_setzero_ps(), _mm_set1_ps(2.0f), 0x4D);
  __m128i h = _mm_cvtps_ph(f, 0);
  __m128i k = _mm_aeskeygenassist_si128(b, 0x1B);
  __m128i c = _mm_clmulepi64_si128(k, h, 0x11);
  __m128i s = _mm_sha1rnds4_epu32(c, a, 2);
  __m128i al = _mm_alignr_epi8(s, a, 5);
  return r + _mm_cvtsi128_si32(al) + _mm256_extract_epi32(g, 0);
}
#endif

// A caller with the features: wrappers inlined, no thunk.
// IR-LABEL: define {{.*}}@"?ext_avx2@@YAHT__m256i@@@Z"
// IR-NOT: openrt.xfeat
// IR: ret i32
__attribute__((target("avx2"))) int ext_avx2(__m256i v) {
  return _mm256_extract_epi32(_mm256_blend_epi32(v, v, 0x5), 1);
}

// ASM-LABEL: "?ext@@YAHPEBH@Z":
// ASM-NOT: ymm
// ASM: call{{.*}}openrt.xfeat
// ASM-LABEL: "?ext_avx2@@YAHT__m256i@@@Z":
// ASM-NOT: call
// ASM: retq

// 32-bit x86: the same wrappers and thunks.
// X86-LABEL: "?ext32@@YAHPBH@Z":
// X86: calll{{.*}}__ms__mm_blend_epi16{{.*}}openrt.xfeat
int ext32(const int *d) {
  __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i *>(d));
  return _mm_extract_epi32(_mm_blend_epi16(a, _mm_setzero_si128(), 0xA5), 2);
}
