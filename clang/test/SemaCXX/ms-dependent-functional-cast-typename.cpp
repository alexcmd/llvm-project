// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -Wno-typename-missing -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only -verify=std %s

// openrt UE-MISSING-TYPENAME (r35): cl takes Dep::Name(args) for a functional
// cast when Name turns out to be a type at instantiation; so does MSVC
// compatibility.
// ms-no-diagnostics

template <class T> struct STableRow {
  struct FArguments { FArguments() {} FArguments(int) {} int x = 0; };
};
template <class T> struct Decl {
  Decl &operator<<=(const typename STableRow<T>::FArguments &) { return *this; }
};
#define SNew(W) Decl<int>() <<= W::FArguments()
template <class ItemType> struct SListView {
  void GenerateNewWidget() {
    auto d = SNew(STableRow<ItemType>); // std-error {{missing 'typename' prior to dependent type name 'STableRow<int>::FArguments'}}
    (void)d;
    auto a = STableRow<ItemType>::FArguments(3); // std-error {{missing 'typename' prior to dependent type name 'STableRow<int>::FArguments'}}
    (void)a;
  }
};
template struct SListView<int>; // std-note {{in instantiation of member function 'SListView<int>::GenerateNewWidget' requested here}}
