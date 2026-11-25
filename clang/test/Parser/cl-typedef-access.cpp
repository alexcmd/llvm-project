// CL-DX-TYPEDEF-ACCESS: cl 19.44 accepts and ignores an access specifier right
// after 'typedef' (MiniEngine DDSTextureLoader.cpp: typedef public
// std::unique_ptr<void, handle_closer> ScopedHandle;), at namespace and class
// scope, silent at /W4 and with /permissive-; after the type it is C2059.

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsyntax-only -verify=std %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 -DONLY_VALID -fsyntax-only -- %s 2>&1 | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /permissive- -DONLY_VALID -fsyntax-only -- %s 2>&1 | count 0

template <class T> struct Ptr { T *p; };

typedef public Ptr<int> P1;  // ms-warning {{access specifier 'public' after 'typedef' is ignored; Microsoft extension}} \
                             // std-error {{expected unqualified-id}}
typedef private int P2;      // ms-warning {{access specifier 'private' after 'typedef'}} \
                             // std-error {{expected unqualified-id}}
typedef protected int P3;    // ms-warning {{access specifier 'protected' after 'typedef'}} \
                             // std-error {{expected unqualified-id}}

struct A {
  typedef public int T;      // ms-warning {{access specifier 'public' after 'typedef'}} \
                             // std-error {{expected member name or ';' after declaration specifiers}}
  T x;                       // std-error {{unknown type name 'T'}}
private:
  int y;
};

#ifndef ONLY_VALID
P1 g1;                       // std-error {{unknown type name 'P1'}}
int typedef public R;        // ms-error {{expected unqualified-id}} \
                             // std-error {{expected unqualified-id}}
public int x;                // ms-error {{expected unqualified-id}} \
                             // std-error {{expected unqualified-id}}
#endif
