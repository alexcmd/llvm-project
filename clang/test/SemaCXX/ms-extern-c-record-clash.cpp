// An #import'ed type library header redeclares SDK GUID variables with its own
// GUID struct inside its namespace; cl accepts that.
// RUN: %clang_cc1 -fsyntax-only -fms-extensions -fms-compatibility -verify=ms %s
// RUN: %clang_cc1 -fsyntax-only -fms-extensions -verify=std %s
// ms-no-diagnostics

typedef struct _GUID { unsigned long Data1; } GUID;
extern "C" const GUID CLSID_StdFont; // std-note {{previous declaration is here}}

namespace stdole {
struct GUID { unsigned long Data1; };
extern "C" const GUID __declspec(selectany) CLSID_StdFont = {1}; // std-error {{redefinition of 'CLSID_StdFont' with a different type}}
}
