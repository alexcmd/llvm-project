// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -fcxx-exceptions -fexceptions -std=c++20 -fsyntax-only -verify %s

// openrt UE-NOTHROW-OVERRIDE (r31): cl 14.51 does not count
// __declspec(nothrow) on the base (COM STDMETHOD_) as an exception
// specification, so an overrider without one is silent; an explicit throw()
// or noexcept on the base still is a mismatch (C2694 in cl).

#define NOTHROW_METHOD_(t, m) virtual __declspec(nothrow) t __stdcall m
struct IVoiceCallback {
  NOTHROW_METHOD_(void, OnPassStart)(unsigned bytes) = 0;
  NOTHROW_METHOD_(void, OnPassEnd)() = 0;
};
struct VoiceCallback : IVoiceCallback {
  void __stdcall OnPassStart(unsigned) override {}
  void __stdcall OnPassEnd() override {}
};

struct B3 { virtual void H() noexcept = 0; }; // expected-note {{overridden virtual function is here}}
struct D3 : B3 { void H() override {} };      // expected-warning {{exception specification of overriding function is more lax than base version}}
