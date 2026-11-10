// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fsyntax-only -verify %s

#pragma fp_contract(off)
#pragma fp_contract(on)
namespace N {
#pragma fp_contract(off)
}
extern "C" {
#pragma fp_contract(on)
}

void local() {
#pragma fp_contract(off) // expected-warning {{'#pragma fp_contract' can only appear at file or namespace scope}}
  int x = 0;
#pragma fp_contract(on) // expected-warning {{'#pragma fp_contract' can only appear at file or namespace scope}}
  (void)x;
  {
#pragma fp_contract(off) // expected-warning {{'#pragma fp_contract' can only appear at file or namespace scope}}
  }
}
struct S {
#pragma fp_contract(on) // expected-warning {{'#pragma fp_contract' can only appear at file or namespace scope}}
};

#pragma fp_contract // expected-error {{expected '('}}
#pragma fp_contract() // expected-error {{pragma fp_contract is malformed}}
#pragma fp_contract(ON) // expected-error {{pragma fp_contract is malformed}}
#pragma fp_contract(default) // expected-error {{pragma fp_contract is malformed}}
#pragma fp_contract(on, push) // expected-error {{expected ')'}}
#pragma fp_contract(off // expected-error {{expected ')'}}
#pragma fp_contract(on) extra // expected-warning {{extra tokens at end of '#pragma fp_contract'}}
