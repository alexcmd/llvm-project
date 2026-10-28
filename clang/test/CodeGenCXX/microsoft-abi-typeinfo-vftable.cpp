// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -emit-llvm -o - %s \
// RUN:   | FileCheck %s --implicit-check-not='type_info@@6B@.'
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -emit-llvm -o - %s \
// RUN:   -DTYPEID_FIRST | FileCheck %s --implicit-check-not='type_info@@6B@.'

// A TU that defines ::type_info's vftable (as the runtime does) must make the
// RTTI type descriptors point at that vftable instead of a second, never
// defined "??_7type_info@@6B@.1", in either emission order.

class type_info {
public:
  virtual ~type_info();
  const char *name() const;

private:
  void *Data;
  char Name[1];
};
namespace std {
using ::type_info;
}

#ifdef TYPEID_FIRST
const type_info &use() { return typeid(int); }
#endif
type_info::~type_info() { __asm__ __volatile__(""); }
#ifndef TYPEID_FIRST
const type_info &use() { return typeid(int); }
#endif

// CHECK: @"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { ptr @"??_7type_info@@6B@", ptr null, [3 x i8] c".H\00" }, comdat
// CHECK: @"??_R0?AVtype_info@@@8" = linkonce_odr global %rtti.TypeDescriptor15 { ptr @"??_7type_info@@6B@", ptr null, [16 x i8] c".?AVtype_info@@\00" }, comdat
// CHECK: @"??_7type_info@@6B@" = alias ptr, getelementptr inbounds ({ [2 x ptr] }, ptr @{{[0-9]+}}, i32 0, i32 0, i32 1)
