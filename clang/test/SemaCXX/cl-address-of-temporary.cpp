// CL-DX-PERMISSIVE: cl 19.44 without /permissive- takes the address of a class
// prvalue (DirectX samples: &CD3DX12_HEAP_PROPERTIES(...)) with C4238 at /W4
// and silently below; with /permissive- it is C2102. clang-cl matches: a
// level-4 warning by default, the error with /permissive- (last of /permissive
// and /permissive- wins) and under /WX; the clang driver keeps the error.

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=W4 %s
// W4: (26,7): warning: taking the address of a temporary object of type 'S' [-Waddress-of-temporary]
// W4: (27,7): warning: taking the address of a temporary object of type 'S' [-Waddress-of-temporary]
// W4-NOT: error

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W3 -fsyntax-only -- %s 2>&1 | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fsyntax-only -- %s 2>&1 | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /wd4238 -fsyntax-only -- %s 2>&1 | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /permissive- /permissive /W4 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=W4 %s

// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /permissive- -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=ERR %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /permissive- /W3 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=ERR %s
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /W4 /WX -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=ERR %s
// RUN: not %clang --target=x86_64-pc-windows-msvc -fsyntax-only %s 2>&1 | FileCheck --check-prefix=ERR %s
// ERR: error: taking the address of a temporary object of type 'S' [-Waddress-of-temporary]

struct S { int v; explicit S(int x) : v(x) {} static S Make(int x) { return S(x); } };
void f(const S*);
void g() {
    f(&S(1));
    f(&S::Make(2));
}
