// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -std=c++17 -O0 -S -o - %s | FileCheck %s --check-prefix=S0
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -std=c++17 -O2 -S -o - %s | FileCheck %s --check-prefix=S2
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -std=c++17 -O2 -fexternc-nounwind -S -o - %s | FileCheck %s --check-prefix=SC2
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -std=c++17 -O0 -fasync-exceptions -S -o - %s | FileCheck %s --check-prefix=A0
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -std=c++17 -O2 -fasync-exceptions -S -o - %s | FileCheck %s --check-prefix=A2
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -std=c++17 -emit-llvm -o - %s | FileCheck %s --check-prefix=IR
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fcxx-exceptions \
// RUN:   -fexceptions -fms-cxx-eh4 -std=c++17 -fasync-exceptions -emit-llvm -o - %s | FileCheck %s --check-prefix=IRA

// openrt FH4-HEADER-PARITY: FuncInfo4 header byte and which maps a record
// has (UnwindMap bit 3, TryBlockMap bit 4, IPtoStateMap always) as cl 14.51
// /d2FH4 emits them, one row per construct (noexcept or not, try, objects with
// destructors), /EHs (S), /EHsc (SC, -fexternc-nounwind), /EHa (A), /Od, /O2.
// Observed: /EHs noexcept or throw() without a try of its own has no unwind
// map (the frame never unwinds); __declspec(nothrow) without a try has no
// record; /EHa keeps the cleanups and still sets NoExcept (bit 6); a record
// without states has a one-entry IPtoStateMap. /O2 /EHs: the inlined ~Obj
// (noexcept, calls a may-throw C function) adds its terminate state, so n_d
// and friends have an unwind map there, as with cl.

extern "C" void note(int);
void may_throw(int);
extern "C" void c_fn(int);

struct Obj {
  int v;
  explicit Obj(int x) : v(x) {}
  ~Obj() { note(v); }
};
static volatile int sink;

void p_d(int x) { Obj a(x); may_throw(x); }
void p_t(int x) { try { may_throw(x); } catch (int) { sink = 1; } }
void p_dt(int x) { Obj a(x); try { Obj b(x); may_throw(x); } catch (int) { sink = 1; } may_throw(x); }
void n_plain(int x) noexcept { may_throw(x); }
void n_d(int x) noexcept { Obj a(x); may_throw(x); }
void n_dc(int x) noexcept { Obj a(x); c_fn(x); }
void n_throw(int x) noexcept { Obj a(x); if (x == 777) throw 1; sink = x; }
void t_d(int x) throw() { Obj a(x); may_throw(x); }
void n_t(int x) noexcept { try { may_throw(x); } catch (int) { sink = 1; } }
void n_dt(int x) noexcept { Obj a(x); try { Obj b(x); may_throw(x); } catch (int) { sink = 1; } may_throw(x); }
void n_d_in(int x) noexcept { try { Obj b(x); may_throw(x); } catch (...) { sink = 1; } }
void n_d_out(int x) noexcept { Obj a(x); try { may_throw(x); } catch (...) { sink = 1; } may_throw(x); }
void n_catch_d(int x) noexcept { try { may_throw(x); } catch (...) { Obj c(x); may_throw(x); } }
__declspec(nothrow) void dn_d(int x) { Obj a(x); may_throw(x); }
__declspec(nothrow) void dn_t(int x) { Obj a(x); try { Obj b(x); may_throw(x); } catch (int) { sink = 1; } may_throw(x); }

// --- -O0 (prefix S0)
// S0-LABEL: "$cppxdata$?p_d@@YAXH@Z":
// S0-NEXT: .byte 40{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?p_d@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$ip2state$?p_d@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?p_t@@YAXH@Z":
// S0-NEXT: .byte 56{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?p_t@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?p_t@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?p_t@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?p_dt@@YAXH@Z":
// S0-NEXT: .byte 56{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?p_dt@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?p_dt@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?p_dt@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_plain@@YAXH@Z":
// S0-NEXT: .byte 96{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$ip2state$?n_plain@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_d@@YAXH@Z":
// S0-NEXT: .byte 96{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$ip2state$?n_d@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_dc@@YAXH@Z":
// S0-NEXT: .byte 96{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$ip2state$?n_dc@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_throw@@YAXH@Z":
// S0-NEXT: .byte 96{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$ip2state$?n_throw@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?t_d@@YAXH@Z":
// S0-NEXT: .byte 96{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$ip2state$?t_d@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_t@@YAXH@Z":
// S0-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?n_t@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?n_t@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?n_t@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_dt@@YAXH@Z":
// S0-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?n_dt@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?n_dt@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?n_dt@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_d_in@@YAXH@Z":
// S0-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?n_d_in@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?n_d_in@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?n_d_in@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_d_out@@YAXH@Z":
// S0-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?n_d_out@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?n_d_out@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?n_d_out@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "$cppxdata$?n_catch_d@@YAXH@Z":
// S0-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?n_catch_d@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?n_catch_d@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?n_catch_d@@YAXH@Z"@IMGREL # IPToStateMap
// S0-LABEL: "?dn_d@@YAXH@Z":
// S0-NOT: .seh_handler
// S0: -- End function
// S0-LABEL: "$cppxdata$?dn_t@@YAXH@Z":
// S0-NEXT: .byte 56{{ +}}# FuncInfo4Header
// S0-NEXT: .long "$stateUnwindMap$?dn_t@@YAXH@Z"@IMGREL # UnwindMap
// S0-NEXT: .long "$tryMap$?dn_t@@YAXH@Z"@IMGREL # TryBlockMap
// S0-NEXT: .long "$ip2state$?dn_t@@YAXH@Z"@IMGREL # IPToStateMap
// --- -O2 (prefix S2)
// S2-LABEL: "$cppxdata$?p_d@@YAXH@Z":
// S2-NEXT: .byte 40{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?p_d@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$ip2state$?p_d@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?p_t@@YAXH@Z":
// S2-NEXT: .byte 56{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?p_t@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?p_t@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?p_t@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?p_dt@@YAXH@Z":
// S2-NEXT: .byte 56{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?p_dt@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?p_dt@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?p_dt@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_plain@@YAXH@Z":
// S2-NEXT: .byte 96{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$ip2state$?n_plain@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_d@@YAXH@Z":
// S2-NEXT: .byte 104{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_d@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$ip2state$?n_d@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_dc@@YAXH@Z":
// S2-NEXT: .byte 104{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_dc@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$ip2state$?n_dc@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_throw@@YAXH@Z":
// S2-NEXT: .byte 104{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_throw@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$ip2state$?n_throw@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?t_d@@YAXH@Z":
// S2-NEXT: .byte 104{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?t_d@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$ip2state$?t_d@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_t@@YAXH@Z":
// S2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_t@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?n_t@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?n_t@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_dt@@YAXH@Z":
// S2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_dt@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?n_dt@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?n_dt@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_d_in@@YAXH@Z":
// S2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_d_in@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?n_d_in@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?n_d_in@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_d_out@@YAXH@Z":
// S2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_d_out@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?n_d_out@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?n_d_out@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?n_catch_d@@YAXH@Z":
// S2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?n_catch_d@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?n_catch_d@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?n_catch_d@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?dn_d@@YAXH@Z":
// S2-NEXT: .byte 40{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?dn_d@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$ip2state$?dn_d@@YAXH@Z"@IMGREL # IPToStateMap
// S2-LABEL: "$cppxdata$?dn_t@@YAXH@Z":
// S2-NEXT: .byte 56{{ +}}# FuncInfo4Header
// S2-NEXT: .long "$stateUnwindMap$?dn_t@@YAXH@Z"@IMGREL # UnwindMap
// S2-NEXT: .long "$tryMap$?dn_t@@YAXH@Z"@IMGREL # TryBlockMap
// S2-NEXT: .long "$ip2state$?dn_t@@YAXH@Z"@IMGREL # IPToStateMap
// --- -O2 -fexternc-nounwind (prefix SC2)
// SC2-LABEL: "$cppxdata$?p_d@@YAXH@Z":
// SC2-NEXT: .byte 40{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?p_d@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$ip2state$?p_d@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?p_t@@YAXH@Z":
// SC2-NEXT: .byte 56{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?p_t@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?p_t@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?p_t@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?p_dt@@YAXH@Z":
// SC2-NEXT: .byte 56{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?p_dt@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?p_dt@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?p_dt@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?n_plain@@YAXH@Z":
// SC2-NEXT: .byte 96{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$ip2state$?n_plain@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?n_d@@YAXH@Z":
// SC2-NEXT: .byte 96{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$ip2state$?n_d@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "?n_dc@@YAXH@Z":
// SC2-NOT: .seh_handler
// SC2: -- End function
// SC2-LABEL: "$cppxdata$?n_throw@@YAXH@Z":
// SC2-NEXT: .byte 96{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$ip2state$?n_throw@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?t_d@@YAXH@Z":
// SC2-NEXT: .byte 96{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$ip2state$?t_d@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?n_t@@YAXH@Z":
// SC2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?n_t@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?n_t@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?n_t@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?n_dt@@YAXH@Z":
// SC2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?n_dt@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?n_dt@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?n_dt@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?n_d_in@@YAXH@Z":
// SC2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?n_d_in@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?n_d_in@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?n_d_in@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?n_d_out@@YAXH@Z":
// SC2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?n_d_out@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?n_d_out@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?n_d_out@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "$cppxdata$?n_catch_d@@YAXH@Z":
// SC2-NEXT: .byte 120{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?n_catch_d@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?n_catch_d@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?n_catch_d@@YAXH@Z"@IMGREL # IPToStateMap
// SC2-LABEL: "?dn_d@@YAXH@Z":
// SC2-NOT: .seh_handler
// SC2: -- End function
// SC2-LABEL: "$cppxdata$?dn_t@@YAXH@Z":
// SC2-NEXT: .byte 56{{ +}}# FuncInfo4Header
// SC2-NEXT: .long "$stateUnwindMap$?dn_t@@YAXH@Z"@IMGREL # UnwindMap
// SC2-NEXT: .long "$tryMap$?dn_t@@YAXH@Z"@IMGREL # TryBlockMap
// SC2-NEXT: .long "$ip2state$?dn_t@@YAXH@Z"@IMGREL # IPToStateMap
// --- -O0 -fasync-exceptions (prefix A0)
// A0-LABEL: "$cppxdata$?p_d@@YAXH@Z":
// A0-NEXT: .byte 8{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?p_d@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$ip2state$?p_d@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?p_t@@YAXH@Z":
// A0-NEXT: .byte 24{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?p_t@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?p_t@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?p_t@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?p_dt@@YAXH@Z":
// A0-NEXT: .byte 24{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?p_dt@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?p_dt@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?p_dt@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_plain@@YAXH@Z":
// A0-NEXT: .byte 64{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$ip2state$?n_plain@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_d@@YAXH@Z":
// A0-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_d@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$ip2state$?n_d@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_dc@@YAXH@Z":
// A0-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_dc@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$ip2state$?n_dc@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_throw@@YAXH@Z":
// A0-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_throw@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$ip2state$?n_throw@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?t_d@@YAXH@Z":
// A0-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?t_d@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$ip2state$?t_d@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_t@@YAXH@Z":
// A0-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_t@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?n_t@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?n_t@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_dt@@YAXH@Z":
// A0-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_dt@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?n_dt@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?n_dt@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_d_in@@YAXH@Z":
// A0-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_d_in@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?n_d_in@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?n_d_in@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_d_out@@YAXH@Z":
// A0-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_d_out@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?n_d_out@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?n_d_out@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?n_catch_d@@YAXH@Z":
// A0-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?n_catch_d@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?n_catch_d@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?n_catch_d@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?dn_d@@YAXH@Z":
// A0-NEXT: .byte 8{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?dn_d@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$ip2state$?dn_d@@YAXH@Z"@IMGREL # IPToStateMap
// A0-LABEL: "$cppxdata$?dn_t@@YAXH@Z":
// A0-NEXT: .byte 24{{ +}}# FuncInfo4Header
// A0-NEXT: .long "$stateUnwindMap$?dn_t@@YAXH@Z"@IMGREL # UnwindMap
// A0-NEXT: .long "$tryMap$?dn_t@@YAXH@Z"@IMGREL # TryBlockMap
// A0-NEXT: .long "$ip2state$?dn_t@@YAXH@Z"@IMGREL # IPToStateMap
// --- -O2 -fasync-exceptions (prefix A2)
// A2-LABEL: "$cppxdata$?p_d@@YAXH@Z":
// A2-NEXT: .byte 8{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?p_d@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$ip2state$?p_d@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?p_t@@YAXH@Z":
// A2-NEXT: .byte 24{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?p_t@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?p_t@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?p_t@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?p_dt@@YAXH@Z":
// A2-NEXT: .byte 24{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?p_dt@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?p_dt@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?p_dt@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_plain@@YAXH@Z":
// A2-NEXT: .byte 64{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$ip2state$?n_plain@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_d@@YAXH@Z":
// A2-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_d@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$ip2state$?n_d@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_dc@@YAXH@Z":
// A2-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_dc@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$ip2state$?n_dc@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_throw@@YAXH@Z":
// A2-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_throw@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$ip2state$?n_throw@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?t_d@@YAXH@Z":
// A2-NEXT: .byte 72{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?t_d@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$ip2state$?t_d@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_t@@YAXH@Z":
// A2-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_t@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?n_t@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?n_t@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_dt@@YAXH@Z":
// A2-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_dt@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?n_dt@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?n_dt@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_d_in@@YAXH@Z":
// A2-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_d_in@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?n_d_in@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?n_d_in@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_d_out@@YAXH@Z":
// A2-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_d_out@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?n_d_out@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?n_d_out@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?n_catch_d@@YAXH@Z":
// A2-NEXT: .byte 88{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?n_catch_d@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?n_catch_d@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?n_catch_d@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?dn_d@@YAXH@Z":
// A2-NEXT: .byte 8{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?dn_d@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$ip2state$?dn_d@@YAXH@Z"@IMGREL # IPToStateMap
// A2-LABEL: "$cppxdata$?dn_t@@YAXH@Z":
// A2-NEXT: .byte 24{{ +}}# FuncInfo4Header
// A2-NEXT: .long "$stateUnwindMap$?dn_t@@YAXH@Z"@IMGREL # UnwindMap
// A2-NEXT: .long "$tryMap$?dn_t@@YAXH@Z"@IMGREL # TryBlockMap
// A2-NEXT: .long "$ip2state$?dn_t@@YAXH@Z"@IMGREL # IPToStateMap

// /EHs: no landing pads where nothing in the function can catch.
// IR-LABEL: define{{.*}} void @"?n_d@@YAXH@Z"
// IR-NOT: invoke
// IR: call void @"?may_throw@@YAXH@Z"
// IR: ret void
// IR-LABEL: define{{.*}} void @"?dn_d@@YAXH@Z"(i32 noundef %x) #{{[0-9]+}} {
// IR-NOT: invoke
// IR: ret void
// __declspec(nothrow) with a try keeps its terminate scope.
// IR-LABEL: define{{.*}} void @"?dn_t@@YAXH@Z"
// IR: unwind label %terminate
// IR: call void @"?terminate@@YAXXZ"()

// /EHa: the noexcept attribute (NoExcept bit) on noexcept functions only.
// IRA: define{{.*}} void @"?n_d@@YAXH@Z"(i32 noundef %x) #[[NOEXC:[0-9]+]] personality
// IRA: invoke void @"?may_throw@@YAXH@Z"
// IRA: define{{.*}} void @"?dn_d@@YAXH@Z"(i32 noundef %x) #[[NOTHROW:[0-9]+]] personality
// IRA-DAG: attributes #[[NOEXC]] = { {{.*}}"msvc-cxx-eh4-noexcept"
// IRA-DAG: attributes #[[NOTHROW]] = { {{[^"]*}}"min-legal-vector-width"="0" "no-trapping-math"
