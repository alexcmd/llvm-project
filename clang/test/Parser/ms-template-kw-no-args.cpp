// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only \
// RUN:   -verify=std %s

// openrt UE-TEMPLATE-KW-NOARGS (nDisplay): cl 14.51 accepts Name::template
// Func(args) without a template argument list (deduced), with or without
// /permissive-; MSVC compatibility does the same. expected-no-diagnostics

struct Conv { template <typename T> static int ToInt(const T &v) { return (int)v; } };
namespace helpers { template <typename T> bool Extract(const T &in, int &out) { out = (int)in; return true; } }
template <typename T> int use(const T &v) {
  int out = 0;
  return Conv::template ToInt(v) + // std-error {{a template argument list is expected after a name prefixed by the template keyword}}
         (helpers::template Extract(v, out) ? out : 0); // std-error {{a template argument list is expected after a name prefixed by the template keyword}}
}
int call() { return use(3); }
