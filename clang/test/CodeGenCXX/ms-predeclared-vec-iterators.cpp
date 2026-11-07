// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-predeclare-eh-types -emit-llvm -o - %s | FileCheck %s --check-prefix=X64
// RUN: %clang_cc1 -triple i686-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fms-predeclare-eh-types -emit-llvm -o - %s | FileCheck %s --check-prefix=X86

// openrt MS-PREDECLARED-RUNTIME-NAMES: cl's predeclared vector iterator
// helpers use the Microsoft ABI names; the eh ones are runtime functions, the
// plain ones are defined in the object (comdat), as with cl.

void use(char *p) {
  __vec_ctor(p, 1, 0, 0);
  __vec_dtor(p, 1, 0, 0);
  __ehvec_ctor(p, 1, 0, 0, 0);
  __ehvec_copy_ctor_vb(p, p, 1, 0, 0, 0);
  __std_terminate();
}

// X64: call void @"??_H@YAXPEAX_K1P6APEAX0@Z@Z"(
// X64: call void @"??_I@YAXPEAX_K1P6AX0@Z@Z"(
// X64: call void @"??_L@YAXPEAX_K1P6AX0@Z2@Z"(
// X64: call void @"??__D@YAXPEAX0_K1P6AX00@ZP6AX0@Z@Z"(
// X64: call void @__std_terminate() #[[NR:[0-9]+]]
// X64: define linkonce_odr dso_local void @"??_H@YAXPEAX_K1P6APEAX0@Z@Z"
// X64: declare dso_local void @"??_L@YAXPEAX_K1P6AX0@Z2@Z"
// X64: attributes #[[NR]] = { noreturn

// X86: call x86_stdcallcc void @"\01??_H@YGXPAXIIP6EPAX0@Z@Z"(
// X86: call x86_stdcallcc void @"\01??_I@YGXPAXIIP6EX0@Z@Z"(
// X86: call x86_stdcallcc void @"\01??_L@YGXPAXIIP6EX0@Z1@Z"(
// X86: call x86_stdcallcc void @"\01??__D@YGXPAX0IIP6EX00@ZP6EX0@Z@Z"(
// X86: define linkonce_odr dso_local x86_stdcallcc void @"\01??_H@YGXPAXIIP6EPAX0@Z@Z"
// X86: call x86_thiscallcc
