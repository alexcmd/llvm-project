// #import attribute handling as cl 14.51: lcid, cl's warnings for unknown or
// short attributes (which also drop every attribute after them),
// inject_statement. stdole2.tlb ships with every Windows.
// REQUIRES: system-windows

// RUN: rm -rf %t && mkdir -p %t/ubt %t/r21 %t/unk %t/inj
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -E -fms-extensions \
// RUN:   -fms-compatibility -isystem %S/Inputs/ms-import-typelib -verify=ok \
// RUN:   -fms-import-output-dir=%t/ubt -DUBT %s -o %t/ubt.i
// RUN: FileCheck %s --check-prefix=RAWNG --input-file=%t/ubt/stdole2.tlh
// RUN: not ls %t/ubt/stdole2.tli
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -E -fms-extensions \
// RUN:   -fms-compatibility -isystem %S/Inputs/ms-import-typelib -verify=ok \
// RUN:   -fms-import-output-dir=%t/r21 -DR21 %s -o %t/r21.i
// RUN: FileCheck %s --check-prefix=RAWNG --input-file=%t/r21/stdole2.tlh
// lcid has no effect on the text of a file import.
// RUN: tail -n +2 %t/ubt/stdole2.tlh | grep -v "^// .*stdole2.tlh" > %t/a.txt
// RUN: tail -n +2 %t/r21/stdole2.tlh | grep -v "^// .*stdole2.tlh" > %t/b.txt
// RUN: diff %t/a.txt %t/b.txt

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -E -fms-extensions \
// RUN:   -fms-compatibility -isystem %S/Inputs/ms-import-typelib -verify=unk \
// RUN:   -fms-import-output-dir=%t/unk -DUNK %s -o %t/unk.i
// RUN: ls %t/unk/stdole2.tli
// RUN: FileCheck %s --check-prefix=UNK --input-file=%t/unk/stdole2.tlh

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -E -fms-extensions \
// RUN:   -fms-compatibility -isystem %S/Inputs/ms-import-typelib -verify=ok \
// RUN:   -fms-import-output-dir=%t/inj -DINJ %s -o %t/inj.i
// RUN: FileCheck %s --check-prefix=INJ --input-file=%t/inj/stdole2.tlh

// ok-no-diagnostics

#if defined(UBT)
// UBT's GenerateTLH stub (VCToolChain.cs), dte80a.olb replaced by stdole2.tlb.
#include <windows.h>
#include <unknwn.h>

#import "C:/Windows/System32/stdole2.tlb" lcid("0") raw_interfaces_only named_guids
#elif defined(R21)
// tests/ue/repros/r21 (import_tlb_lcid.cpp)
#import "C:/Windows/System32/stdole2.tlb" lcid("0") raw_interfaces_only named_guids
int tlh_probe_lcid() { return 0; }
#elif defined(UNK)
#import "C:/Windows/System32/stdole2.tlb" bogus_attr raw_interfaces_only named_guids // unk-warning {{ignoring unknown #import attribute 'bogus_attr'}}
#elif defined(INJ)
#import "C:/Windows/System32/stdole2.tlb" raw_interfaces_only inject_statement("int injected_x;")
#endif

// RAWNG: namespace stdole {
// RAWNG: extern "C" const GUID __declspec(selectany) LIBID_stdole =

// UNK: namespace stdole {
// UNK-NOT: LIBID_stdole
// UNK: #include "{{.*}}stdole2.tli"

// INJ: namespace stdole {
// INJ-EMPTY:
// INJ-NEXT: //
// INJ-NEXT: // User-injected statements
// INJ-NEXT: //
// INJ-EMPTY:
// INJ-NEXT: int injected_x;
// INJ-EMPTY:
// INJ-NEXT: //
// INJ-NEXT: // Forward references and typedefs
