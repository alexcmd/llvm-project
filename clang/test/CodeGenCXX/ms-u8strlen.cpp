// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -emit-llvm -o - %s | FileCheck %s

static_assert(__builtin_u8strlen(u8"") == 0);
static_assert(__builtin_u8strlen(u8"abc") == 3);
static_assert(__builtin_u8strlen(u8"a\0bc") == 1);
static_assert(__builtin_u8strlen(u8"\u00e9") == 2);
constexpr const char8_t *text = u8"abcd";
static_assert(__builtin_u8strlen(text + 1) == 3);
static_assert(__is_same(decltype(__builtin_u8strlen(u8"x")), __SIZE_TYPE__));
// CHECK-LABEL: define {{.*}}@"?length@@
// CHECK: call i64 @strlen(ptr {{.*}})
__SIZE_TYPE__ length(const char8_t *p) { return __builtin_u8strlen(p); }
