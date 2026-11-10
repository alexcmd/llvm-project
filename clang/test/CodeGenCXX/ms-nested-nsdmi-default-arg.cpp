// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -emit-llvm -o - %s | FileCheck %s

int initialize();
struct Outer {
  struct Leaf { int value = initialize(); };
  struct Member { Leaf leaf; };
  struct Base : Member {};
  void trigger(Base value = Base());
};

int read() { Outer::Base value; return value.leaf.value; }

// Referencing Leaf through the containing constructor before its initializer
// is parsed must still produce its definition. No later direct use of Leaf
// requests it again; omitting it leaves an unresolved constructor at link time.
// CHECK: define {{.*}}@"??0Leaf@Outer@@QEAA@XZ"
// CHECK: call {{.*}}@"?initialize@@YAHXZ"
