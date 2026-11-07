// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions -std=c++14 \
// RUN:   -fsyntax-only -verify -fms-predeclare-eh-types %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions -std=c++17 -DCXX17 \
// RUN:   -fsyntax-only -verify=expected,cxx17 -fms-predeclare-eh-types %s
// RUN: %clang_cc1 -triple i686-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fcxx-exceptions -fexceptions -std=c++20 -DCXX17 \
// RUN:   -fsyntax-only -verify=expected,cxx17 -fms-predeclare-eh-types %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -fsyntax-only -verify=c -x c \
// RUN:   -fms-predeclare-eh-types %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /Zs /std:c++latest -DCXX17 -Xclang -verify=expected,cxx17 -- %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /Zs /TC -Xclang -verify=c -- %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /Zs -fno-ms-predeclare-eh-types -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=OFF %s
// OFF: use of undeclared identifier '__std_terminate'

// openrt MS-PREDECLARED-RUNTIME-NAMES: the names cl 14.51 declares in every
// translation unit without any header (address-of, sizeof, redeclaration,
// noexcept and noreturn probes), with its signatures.

#ifdef __cplusplus
int f1() { __std_terminate(); } // noreturn: no missing-return warning
int f2() { _CxxThrowException(0, 0); }
int f3() { __report_rangecheckfailure(); }
int f4() { __report_securityfailure(1); }
int f5() { __report_securityfailureEx(1, 2, 0); }

// extern "C" redeclarations with the same type are compatible.
extern "C" void __std_terminate(void);
extern "C" int __cdecl _purecall(void);
extern "C" void *__cdecl __RTDynamicCast(void *, long, void *, void *, int) noexcept(false);
extern "C" void *__cdecl __RTtypeid(void *) noexcept(false);
extern "C" void *__cdecl __RTCastToVoid(void *) noexcept(false);
extern "C" void __CxxUnregisterExceptionObject(void *, int);
extern "C" int __CxxRegisterExceptionObject(void *, void *);
extern "C" int __CxxDetectRethrow(void *);
extern "C" int __CxxExceptionFilter(void *, void *, int, void *);
extern "C" int __CxxQueryExceptionSize(void);
extern "C" int __cdecl atexit(void(__cdecl *)(void));
extern "C" void _Init_thread_header(int *) noexcept;
extern "C" void _Init_thread_footer(int *) noexcept;
extern "C" void _Init_thread_abort(int *) noexcept;
extern "C" void __dyn_tls_on_demand_init(void) noexcept;
extern "C" __declspec(thread) int _Init_thread_epoch;
extern "C" __declspec(thread) bool __tls_guard;

// cl also accepts a redeclaration or definition without the noexcept of a
// predeclared function (the openrt runtime defines them that way); it stays
// noexcept.
extern "C" void __cdecl _Init_thread_header(int *const);
extern "C" void __cdecl _Init_thread_abort(int *const) {}

// Exception specifications as cl has them.
static_assert(noexcept(_Init_thread_header(0)), "");
static_assert(noexcept(__dyn_tls_on_demand_init()), "");
static_assert(!noexcept(__RTCastToVoid(0)), "");
static_assert(!noexcept(__RTDynamicCast(0, 0, 0, 0, 0)), "");

// Language linkage: extern "C".
extern "C++" void __std_terminate(); // expected-error {{declaration of '__std_terminate' has a different language linkage}}
                        // expected-note@* {{previous declaration is here}}

// Vector iterator helpers.
void iters(char *p) {
  __vec_ctor(p, 1, 0, 0);
  __vec_dtor(p, 1, 0, 0);
  __vec_ctor_vb(p, 1, 0, 0);
  __vec_copy_ctor(p, p, 1, 0, 0);
  __vec_copy_ctor_vb(p, p, 1, 0, 0);
  __ehvec_ctor(p, 1, 0, 0, 0);
  __ehvec_dtor(p, 1, 0, 0);
  __ehvec_ctor_vb(p, 1, 0, 0, 0);
  __ehvec_copy_ctor(p, p, 1, 0, 0, 0);
  __ehvec_copy_ctor_vb(p, p, 1, 0, 0, 0);
}

std::nullptr_t np = nullptr;
static_assert(sizeof(__s_GUID) == 16 && alignof(__s_GUID) == 4, "");
unsigned long *d1(__s_GUID &g) { return &g.Data1; }
struct _GUID *gp; // still only declared

#ifdef CXX17
static_assert(sizeof(std::align_val_t) == sizeof(void *), "");
void *pa = operator new(16, std::align_val_t(16));
namespace std { enum class align_val_t : __SIZE_TYPE__ {}; } // <new> defines it
#else
std::align_val_t *pa; // expected-error {{no type named 'align_val_t' in namespace 'std'}}
#endif
#else
// C: only the failure-report functions.
int c1(void) { __report_rangecheckfailure(); }
int c2(void) { __report_securityfailure(1); }
int c3(void) { __report_securityfailureEx(1, 2, 0); }
void *c4 = (void *)&__std_terminate; // c-error {{use of undeclared identifier '__std_terminate'}}
#endif
