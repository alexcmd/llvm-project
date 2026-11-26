// CL-DX-NONPOD-VARARG: cl 19.44 passes a class object of non-trivial type
// through '...' as a bitwise copy: the copy constructor is checked but not
// called, the copy is not destroyed, a prvalue is materialized and destroyed
// at the end of the full-expression; C4840 (level 4) only for a non-trivial
// copy or move constructor (observed). clang's -Wnon-pod-varargs error stays
// outside cl mode.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -std=c++17 -Wmicrosoft-non-pod-varargs -fsyntax-only -verify=cl %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++17 -fsyntax-only -verify=std %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fno-ms-clang-identity -std=c++17 -DNO_ERRORS -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W4 /std:c++17 -DNO_ERRORS -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=W4 %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /W3 /std:c++17 -DNO_ERRORS -fsyntax-only -- %s 2>&1 >/dev/null | count 0

// W4: warning: non-portable use of class 'C' as an argument to a variadic function; its constructor and destructor are not called, a bitwise copy of the object is passed [-Wmicrosoft-non-pod-varargs]
// W4-NOT: class 'D'

struct D { int v; D(); ~D(); };                  // non-trivial destructor only
struct C { int v; C(); C(const C &); ~C(); };    // non-trivial copy
struct B { long long a, b; B(); B(const B &); }; // 16 bytes
struct M { int v; M(); M(M &&); };               // #mdecl

void f(int, ...);
extern "C" int printf(const char *, ...);
C mkc();

// 4 bytes: in a register, no copy constructor, the copy is not destroyed.
// CHECK-LABEL: define {{.*}}@"?g@@YAXXZ"
// CHECK-NOT: call {{.*}}@"??0C@@QEAA@AEBU0@@Z"
// CHECK: %[[CV:.*]] = load i32, ptr %{{.*}}
// CHECK-NOT: call {{.*}}@"??0C@@QEAA@AEBU0@@Z"
// CHECK: call void (i32, ...) @"?f@@YAXHZZ"(i32 noundef 1, i32 {{.*}}%[[CV]])
// CHECK: call void @"??1C@@QEAA@XZ"
// CHECK-NOT: @"??1C@@QEAA@XZ"
// CHECK: ret void
void g() {
  C c;
  f(1, c); // cl-warning {{non-portable use of class 'C'}} std-error {{cannot pass object of non-trivial type 'C'}}
}

// 16 bytes: a pointer to a bitwise copy, no copy constructor.
// CHECK-LABEL: define {{.*}}@"?h@@YAXXZ"
// CHECK-NOT: call {{.*}}@"??0B@@QEAA@AEBU0@@Z"
// CHECK: call void @llvm.memcpy{{.*}}(ptr {{.*}}%[[TMP:[^,]+]], ptr {{.*}}, i64 16, i1 false)
// CHECK-NOT: call {{.*}}@"??0B@@QEAA@AEBU0@@Z"
// CHECK: call void (i32, ...) @"?f@@YAXHZZ"(i32 noundef 1, ptr {{.*}}%[[TMP]])
void h() {
  B b;
  f(1, b); // cl-warning {{non-portable use of class 'B'}} std-error {{cannot pass object of non-trivial type 'B'}}
}

// A non-trivial destructor alone is silent; a prvalue is destroyed after the
// call, the copy of a glvalue never.
// CHECK-LABEL: define {{.*}}@"?k@@YAXXZ"
// CHECK: call void (i32, ...) @"?f@@YAXHZZ"(i32 noundef 1,
// CHECK-NOT: @"??1D@@QEAA@XZ"
// CHECK: call void (i32, ...) @"?f@@YAXHZZ"(i32 noundef 2,
// CHECK-NEXT: call void @"??1D@@QEAA@XZ"
// CHECK-NEXT: call void @"??1D@@QEAA@XZ"
// CHECK-NEXT: ret void
void k() {
  D d;
  f(1, d); // std-error {{cannot pass object of non-trivial type 'D'}}
  f(2, D()); // std-error {{cannot pass object of non-trivial type 'D'}}
}

// A prvalue of a non-trivially-copyable class: materialized, copied bitwise,
// destroyed after the call.
// CHECK-LABEL: define {{.*}}@"?m@@YAXXZ"
// CHECK: call void @"?mkc@@YA?AUC@@XZ"(ptr {{.*}}%[[T:[^)]+]])
// CHECK: call void @llvm.memcpy{{.*}}(ptr {{.*}}, ptr {{.*}}%[[T]], i64 4, i1 false)
// CHECK: call void (i32, ...) @"?f@@YAXHZZ"
// CHECK-NEXT: call void @"??1C@@QEAA@XZ"(ptr {{.*}}%[[T]])
void m() {
  f(1, mkc()); // cl-warning {{non-portable use of class 'C'}} std-error {{cannot pass object of non-trivial type 'C'}}
}

#ifndef NO_ERRORS
// The format checker sees the class, as for any other class argument.
void p(C c) {
  printf("%d", c); // cl-warning {{non-portable use of class 'C'}} cl-warning {{format specifies type 'int' but the argument has type 'C'}} std-error {{cannot pass non-trivial object of type 'C' to variadic function; expected type from format string was 'int'}}
}

// Deleted copy: an error, as in cl (C4839, C2280).
void q(M m) {
  f(1, m); // cl-error {{call to implicitly-deleted copy constructor of 'M'}} std-error {{call to implicitly-deleted copy constructor of 'M'}}
}
// cl-note@#mdecl {{copy constructor is implicitly deleted because 'M' has a user-declared move constructor}}
// std-note@#mdecl {{copy constructor is implicitly deleted because 'M' has a user-declared move constructor}}
#endif
