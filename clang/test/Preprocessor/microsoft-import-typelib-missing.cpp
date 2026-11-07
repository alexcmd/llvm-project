// RUN: not %clang_cc1 -E -fms-compatibility -fms-import-output-dir=%t %s 2>&1 | FileCheck %s
// REQUIRES: system-windows

#import "no-such-typelib.tlb" raw_interfaces_only
// CHECK: fatal error: cannot open type library file: 'no-such-typelib.tlb': No such file or directory
