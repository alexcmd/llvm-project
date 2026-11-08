// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -O1 -disable-llvm-passes -emit-llvm -o - %s | FileCheck %s

// openrt CL-IDENTITY: the cl intrinsics the cl branches of its headers call.
// Same code as cl 14.51 /O2: libm-style rounding (llvm.ceil, ...), one
// divq/idivq for _udiv128/_div128, tzcnt, padding cleared by
// __builtin_zero_non_value_bits (also usable in constant evaluation).

extern "C" {
unsigned __int64 _udiv128(unsigned __int64, unsigned __int64, unsigned __int64,
                          unsigned __int64 *);
__int64 _div128(__int64, __int64, __int64, __int64 *);
unsigned int _tzcnt_u32(unsigned int);
unsigned __int64 _tzcnt_u64(unsigned __int64);
}

// CHECK-LABEL: define {{.*}}@fceilf
// CHECK: call float @llvm.ceil.f32
extern "C" float fceilf(float x) { return __ceilf(x); }
// CHECK-LABEL: define {{.*}}@fceil
// CHECK: call double @llvm.ceil.f64
extern "C" double fceil(double x) { return __ceil(x); }
// CHECK-LABEL: define {{.*}}@ffloorf
// CHECK: call float @llvm.floor.f32
extern "C" float ffloorf(float x) { return __floorf(x); }
// CHECK-LABEL: define {{.*}}@ffloor
// CHECK: call double @llvm.floor.f64
extern "C" double ffloor(double x) { return __floor(x); }
// CHECK-LABEL: define {{.*}}@froundf
// CHECK: call float @llvm.round.f32
extern "C" float froundf(float x) { return __roundf(x); }
// CHECK-LABEL: define {{.*}}@fround
// CHECK: call double @llvm.round.f64
extern "C" double fround(double x) { return __round(x); }
// CHECK-LABEL: define {{.*}}@ftruncf
// CHECK: call float @llvm.trunc.f32
extern "C" float ftruncf(float x) { return __truncf(x); }
// CHECK-LABEL: define {{.*}}@ftrunc
// CHECK: call double @llvm.trunc.f64
extern "C" double ftrunc(double x) { return __trunc(x); }
// CHECK-LABEL: define {{.*}}@fcopysignf
// CHECK: call float @llvm.copysign.f32
extern "C" float fcopysignf(float x, float y) { return __copysignf(x, y); }
// CHECK-LABEL: define {{.*}}@fcopysign
// CHECK: call double @llvm.copysign.f64
extern "C" double fcopysign(double x, double y) { return __copysign(x, y); }

// CHECK-LABEL: define {{.*}}@fudiv
// CHECK: call { i64, i64 } asm "divq $4", "={ax},={dx},{dx},{ax},r,~{dirflag},~{fpsr},~{flags}"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
// CHECK: store i64 %{{.*}}, ptr %{{.*}}, align 8
extern "C" unsigned __int64 fudiv(unsigned __int64 h, unsigned __int64 l,
                                  unsigned __int64 d, unsigned __int64 *r) {
  return _udiv128(h, l, d, r);
}
// CHECK-LABEL: define {{.*}}@fdiv
// CHECK: call { i64, i64 } asm "idivq $4"
extern "C" __int64 fdiv(__int64 h, __int64 l, __int64 d, __int64 *r) {
  return _div128(h, l, d, r);
}

// CHECK-LABEL: define {{.*}}@ftz
// CHECK: call i32 @llvm.cttz.i32(i32 %{{.*}}, i1 false)
extern "C" unsigned ftz(unsigned x) { return _tzcnt_u32(x); }
// CHECK-LABEL: define {{.*}}@ftz64
// CHECK: call i64 @llvm.cttz.i64(i64 %{{.*}}, i1 false)
extern "C" unsigned __int64 ftz64(unsigned __int64 x) { return _tzcnt_u64(x); }

struct P { char c; int i; };
// CHECK-LABEL: define {{.*}}@fz
// CHECK: store i8 0
extern "C" void fz(P *p) { ::__builtin_zero_non_value_bits(p); }

constexpr int h() { P p{1, 2}; __builtin_zero_non_value_bits(&p); return p.i; }
static_assert(h() == 2);
