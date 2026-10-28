// REQUIRES: x86-registered-target
// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=19.33 -triple x86_64-pc-windows-msvc -O2 -S %s -o - | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -fms-extensions -fms-compatibility -fms-compatibility-version=19.33 -triple i686-pc-windows-msvc -O2 -S %s -o - | FileCheck %s

// __stosb must be "rep stosb", not a memset call: a memset implemented with
// __stosb would otherwise call itself (GH#33522).

typedef __SIZE_TYPE__ size_t;
#include <intrin.h>

void *memset(void *d, int c, size_t n) {
  __stosb((unsigned char *)d, (unsigned char)c, n);
  return d;
}
// CHECK-LABEL: memset:
// CHECK-NOT:   {{call|jmp}}{{.*}}memset
// CHECK:       rep{{.*}}stosb
// CHECK-NOT:   {{call|jmp}}{{.*}}memset
// CHECK:       ret
