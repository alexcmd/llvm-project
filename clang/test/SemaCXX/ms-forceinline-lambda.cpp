// RUN: %clang_cc1 -fsyntax-only -fms-extensions -verify=quiet %s
// RUN: %clang_cc1 -fsyntax-only -fms-extensions -pedantic -verify=ms %s
// RUN: %clang_cc1 -fsyntax-only -fms-extensions -Wmicrosoft-lambda-attribute -verify=ms %s
// RUN: %clang_cc1 -fsyntax-only -fms-extensions -Wmicrosoft -verify=ms %s
// RUN: %clang_cc1 -fsyntax-only -verify=noms %s

// cl accepts [[msvc::forceinline]] after the lambda declarator (the MSVC STL
// uses it) and applies it to operator(). Like cl, this is silent by default;
// the diagnostic is an Extension shown only with -pedantic or
// -Wmicrosoft[-lambda-attribute].
// quiet-no-diagnostics
int f(int y) {
  auto a = [](int x) noexcept [[msvc::forceinline]] { return x + 1; };
  // ms-warning@-1 {{'msvc::forceinline' after the lambda declarator is a Microsoft extension; it applies to the call operator}}
  // noms-error@-2 {{'msvc::forceinline' attribute cannot be applied to types}}
  auto b = [](int x) [[msvc::forceinline]] -> int { return x; };
  // ms-warning@-1 {{'msvc::forceinline' after the lambda declarator is a Microsoft extension; it applies to the call operator}}
  // noms-error@-2 {{'msvc::forceinline' attribute cannot be applied to types}}
  return a(y) + b(y);
}
