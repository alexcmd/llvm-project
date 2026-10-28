# A COMDAT section may be named after its own key symbol, as in msvcrt's
# cfg_fo.obj. The name must keep referring to the COMDAT symbol (so the label
# defines it) while the section gets its own static section symbol.

# RUN: llvm-mc -triple x86_64-windows-msvc -filetype=obj %s -o %t.obj
# RUN: llvm-readobj --sections --symbols --relocations %t.obj | FileCheck %s

.section "_guard_check_icall_$fo_bdd$","dr",discard,"_guard_check_icall_$fo_bdd$"
.globl "_guard_check_icall_$fo_bdd$"
.globl "_guard_check_icall_$fo$"
"_guard_check_icall_$fo_bdd$":
"_guard_check_icall_$fo$":
.long 1, 0x18, 0x00010002, 0x141, 0x00010001, 0, 0, 0

# The key symbol may also be declared before the section.
.globl "_guard_check_icall_$fo_rvas$"
.section "_guard_check_icall_$fo_rvas$","dr",discard,"_guard_check_icall_$fo_rvas$"
"_guard_check_icall_$fo_rvas$":
.long _guard_check_icall@IMGREL

# A reference by name binds to the external symbol (index 8: .text, .data,
# .bss and the section symbol each take two entries), not to the section.
.data
.quad "_guard_check_icall_$fo_bdd$"

# CHECK:      Name: _guard_check_icall_$fo_bdd$ (
# CHECK:      RawDataSize: 32
# CHECK:        IMAGE_SCN_LNK_COMDAT (0x1000)
# CHECK:      Name: _guard_check_icall_$fo_rvas$ (
# CHECK:      RawDataSize: 4
# CHECK:        IMAGE_SCN_LNK_COMDAT (0x1000)

# CHECK:      Relocations [
# CHECK-NEXT:   Section (2) .data {
# CHECK-NEXT:     0x0 IMAGE_REL_AMD64_ADDR64 _guard_check_icall_$fo_bdd$ (8)
# CHECK-NEXT:   }
# CHECK-NEXT:   Section ([[RVAS:[0-9]+]]) _guard_check_icall_$fo_rvas$ {
# CHECK-NEXT:     0x0 IMAGE_REL_AMD64_ADDR32NB _guard_check_icall ({{[0-9]+}})
# CHECK-NEXT:   }
# CHECK-NEXT: ]

# CHECK:        Name: _guard_check_icall_$fo_bdd${{$}}
# CHECK-NEXT:   Value: 0
# CHECK-NEXT:   Section: _guard_check_icall_$fo_bdd$ ([[BDD:[0-9]+]])
# CHECK-NEXT:   BaseType: Null (0x0)
# CHECK-NEXT:   ComplexType: Null (0x0)
# CHECK-NEXT:   StorageClass: Static (0x3)
# CHECK-NEXT:   AuxSymbolCount: 1
# CHECK-NEXT:   AuxSectionDef {
# CHECK-NEXT:     Length: 32
# CHECK-NEXT:     RelocationCount: 0
# CHECK-NEXT:     LineNumberCount: 0
# CHECK-NEXT:     Checksum: 0x{{[0-9A-F]+}}
# CHECK-NEXT:     Number: [[BDD]]
# CHECK-NEXT:     Selection: Any (0x2)
# CHECK-NEXT:   }
# CHECK-NEXT: }
# CHECK-NEXT: Symbol {
# CHECK-NEXT:   Name: _guard_check_icall_$fo_bdd$
# CHECK-NEXT:   Value: 0
# CHECK-NEXT:   Section: _guard_check_icall_$fo_bdd$ ([[BDD]])
# CHECK-NEXT:   BaseType: Null (0x0)
# CHECK-NEXT:   ComplexType: Null (0x0)
# CHECK-NEXT:   StorageClass: External (0x2)
# CHECK-NEXT:   AuxSymbolCount: 0
# CHECK-NEXT: }
# CHECK-NEXT: Symbol {
# CHECK-NEXT:   Name: _guard_check_icall_$fo_rvas$
# CHECK-NEXT:   Value: 0
# CHECK-NEXT:   Section: _guard_check_icall_$fo_rvas$ ([[RVAS]])
# CHECK-NEXT:   BaseType: Null (0x0)
# CHECK-NEXT:   ComplexType: Null (0x0)
# CHECK-NEXT:   StorageClass: Static (0x3)
# CHECK-NEXT:   AuxSymbolCount: 1
# CHECK-NEXT:   AuxSectionDef {
# CHECK-NEXT:     Length: 4
# CHECK-NEXT:     RelocationCount: 1
# CHECK-NEXT:     LineNumberCount: 0
# CHECK-NEXT:     Checksum: 0x{{[0-9A-F]+}}
# CHECK-NEXT:     Number: [[RVAS]]
# CHECK-NEXT:     Selection: Any (0x2)
# CHECK-NEXT:   }
# CHECK-NEXT: }
# CHECK-NEXT: Symbol {
# CHECK-NEXT:   Name: _guard_check_icall_$fo_rvas$
# CHECK-NEXT:   Value: 0
# CHECK-NEXT:   Section: _guard_check_icall_$fo_rvas$ ([[RVAS]])
# CHECK-NEXT:   BaseType: Null (0x0)
# CHECK-NEXT:   ComplexType: Null (0x0)
# CHECK-NEXT:   StorageClass: External (0x2)
# CHECK-NEXT:   AuxSymbolCount: 0
# CHECK-NEXT: }
# CHECK:      Name: _guard_check_icall_$fo$
# CHECK-NEXT:   Value: 0
# CHECK-NEXT:   Section: _guard_check_icall_$fo_bdd$ ([[BDD]])
# CHECK:        StorageClass: External (0x2)
