// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions -fexceptions -fsyntax-only -Wreturn-type -verify %s

// A C++ throw inside __try makes the CFG edge to the (empty) SEH dispatch
// block, whose successors include the exit block. That edge is abnormal and
// must not count as falling off the end of the function.


struct E {};
int g();
void *find(void *);

int f1() {
  int r;
  __try {
    r = g();
    if (!r)
      throw E();
  } __except (1) {
    r = -1;
  }
  return r;
}

void *f2(void *p) noexcept(false) {
  if (p == nullptr)
    throw E();
  void *r;
  __try {
    r = find(p);
    if (r == nullptr)
      throw E();
  } __except (1) {
    throw E();
  }
  return r;
}

// Falling off the end of the __except block is still diagnosed.
int f3(int x) {
  __try {
    if (x)
      throw E();
    return 1;
  } __except (1) {
  }
} // expected-warning {{non-void function does not return a value in all control paths}}
