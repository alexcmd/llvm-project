// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -ffreestanding -fsyntax-only -verify \
// RUN:   -include __msvc_eh_predef.h %s
// RUN: %clang_cc1 -triple aarch64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -ffreestanding -fsyntax-only -verify \
// RUN:   -include __msvc_eh_predef.h %s
// RUN: %clang_cc1 -triple i686-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -ffreestanding -fsyntax-only -verify \
// RUN:   -include __msvc_eh_predef.h %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -ffreestanding -fsyntax-only -verify=c -x c \
// RUN:   -include __msvc_eh_predef.h %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fms-predeclare-eh-types \
// RUN:   /Zs -Xclang -verify -- %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -ffreestanding -emit-llvm -o - \
// RUN:   -include __msvc_eh_predef.h %s | FileCheck --check-prefix=X64 %s
// RUN: %clang_cc1 -triple i686-pc-windows-msvc -fms-extensions \
// RUN:   -fms-compatibility -ffreestanding -emit-llvm -o - \
// RUN:   -include __msvc_eh_predef.h %s | FileCheck --check-prefix=X86 %s

// Layouts, field types and tag names of the C++ EH types cl.exe pre-injects,
// as observed with cl.exe 19.44 (x86 and x64): pointer fields on every target.

#ifndef __cplusplus
// cl.exe does not pre-inject them in C.
_ThrowInfo *p; // c-error {{unknown type name '_ThrowInfo'}}
#else
// expected-no-diagnostics
#define offsetof __builtin_offsetof

template <class A, class B> struct same { static const bool v = false; };
template <class A> struct same<A, A> { static const bool v = true; };
#define SAME(a, b) static_assert(same<a, b>::v, #a)
#define P sizeof(void *)

static_assert(sizeof(_PMD) == 12, "");
static_assert(offsetof(_PMD, pdisp) == 4 && offsetof(_PMD, vdisp) == 8, "");
static_assert(sizeof(_TypeDescriptor) == 2 * P, "");
static_assert(alignof(_TypeDescriptor) == P, "");
static_assert(offsetof(_TypeDescriptor, spare) == P, "");
static_assert(offsetof(_TypeDescriptor, name) == 2 * P, "");
static_assert(sizeof(_CatchableType) == 20 + 2 * P, "");
static_assert(alignof(_CatchableType) == 4, "");
static_assert(offsetof(_CatchableType, pType) == 4, "");
static_assert(offsetof(_CatchableType, thisDisplacement) == 4 + P, "");
static_assert(offsetof(_CatchableType, sizeOrOffset) == 16 + P, "");
static_assert(offsetof(_CatchableType, copyFunction) == 20 + P, "");
static_assert(sizeof(_CatchableTypeArray) == 4, "");
static_assert(offsetof(_CatchableTypeArray, arrayOfCatchableTypes) == 4, "");
static_assert(sizeof(_ThrowInfo) == 4 + 3 * P, "");
static_assert(alignof(_ThrowInfo) == 4, "");
static_assert(offsetof(_ThrowInfo, pmfnUnwind) == 4, "");
static_assert(offsetof(_ThrowInfo, pForwardCompat) == 4 + P, "");
static_assert(offsetof(_ThrowInfo, pCatchableTypeArray) == 4 + 2 * P, "");

SAME(_PMFN, void(__cdecl *)(void *));
SAME(decltype(_ThrowInfo::attributes), unsigned int);
SAME(decltype(_ThrowInfo::pmfnUnwind), _PMFN);
SAME(decltype(_ThrowInfo::pForwardCompat), int(__cdecl *)(...));
SAME(decltype(_ThrowInfo::pCatchableTypeArray), _CatchableTypeArray *);
SAME(decltype(((_CatchableTypeArray *)0)->arrayOfCatchableTypes[0]),
     _CatchableType *const &);
SAME(decltype(_CatchableType::properties), unsigned int);
SAME(decltype(_CatchableType::pType), _TypeDescriptor *);
SAME(decltype(_CatchableType::thisDisplacement), _PMD);
SAME(decltype(_CatchableType::copyFunction), _PMFN);
SAME(decltype(_TypeDescriptor::pVFTable), const void *);
SAME(decltype(_TypeDescriptor::spare), void *);
SAME(_ThrowInfo, const struct _s__ThrowInfo);
SAME(_CatchableType, const struct _s__CatchableType);
SAME(_CatchableTypeArray, const struct _s__CatchableTypeArray);
SAME(_PMD, struct _PMD);
SAME(_TypeDescriptor, struct _TypeDescriptor);

// MSVC's <ehdata_forceinclude.h> declares this with the pre-injected type.
extern "C" __declspec(noreturn) void __stdcall _CxxThrowException(
    void *pExceptionObject, _ThrowInfo *pThrowInfo) noexcept(false);

// Mangled names match cl.exe.
void f1(_ThrowInfo *) {}
void f2(_CatchableType *) {}
void f3(_CatchableTypeArray *) {}
void f4(_PMD *) {}
void f5(_TypeDescriptor *) {}
void f6(_PMFN) {}
// X64: define {{.*}}@"?f1@@YAXPEBU_s__ThrowInfo@@@Z"
// X64: define {{.*}}@"?f2@@YAXPEBU_s__CatchableType@@@Z"
// X64: define {{.*}}@"?f3@@YAXPEBU_s__CatchableTypeArray@@@Z"
// X64: define {{.*}}@"?f4@@YAXPEAU_PMD@@@Z"
// X64: define {{.*}}@"?f5@@YAXPEAU_TypeDescriptor@@@Z"
// X64: define {{.*}}@"?f6@@YAXP6AXPEAX@Z@Z"
// X86: define {{.*}}@"?f1@@YAXPBU_s__ThrowInfo@@@Z"
// X86: define {{.*}}@"?f2@@YAXPBU_s__CatchableType@@@Z"
// X86: define {{.*}}@"?f3@@YAXPBU_s__CatchableTypeArray@@@Z"
// X86: define {{.*}}@"?f4@@YAXPAU_PMD@@@Z"
// X86: define {{.*}}@"?f5@@YAXPAU_TypeDescriptor@@@Z"
// X86: define {{.*}}@"?f6@@YAXP6AXPAX@Z@Z"
#endif
