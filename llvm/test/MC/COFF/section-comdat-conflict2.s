// RUN: not llvm-mc -triple i386-pc-win32 -filetype=obj < %s 2>&1 |  FileCheck %s

// CHECK: error: two sections have the same comdat 'bar'

        .section        .xyz,"xr",discard,bar
        .section        .abcd,"xr",discard,bar
