// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple i686-pc-windows-msvc -fms-extensions -fms-compatibility -std=c++20 -fsyntax-only -verify %s

struct Record { int first; int second[3]; };
struct Outer { int prefix; Record inner; };
struct NonPOD { virtual ~NonPOD(); int member; };
#define OFFSET(T, F) (reinterpret_cast<const char*>(&reinterpret_cast<const T*>(16)->F) - reinterpret_cast<const char*>(16))
static_assert(OFFSET(Record, second) == 4);
static_assert(OFFSET(Outer, inner.second) == 8);
static_assert(OFFSET(NonPOD, member) == __builtin_offsetof(NonPOD, member)); // expected-warning {{'offsetof' on non-standard-layout type 'NonPOD'}}
template<class T> constexpr unsigned offset() { return OFFSET(T, second); }
static_assert(offset<Record>() == 4);
struct Metadata { unsigned short offset; };
constexpr Metadata metadata{static_cast<unsigned>(OFFSET(Record, second))};
static_assert(metadata.offset == 4);

constexpr auto indexed = OFFSET(Record, second[1]); // expected-error {{must be initialized by a constant expression}} expected-note {{reinterpret_cast is not allowed in a constant expression}}
constexpr auto mismatched = reinterpret_cast<const char*>(&reinterpret_cast<const Record*>(16)->second) - reinterpret_cast<const char*>(32); // expected-error {{must be initialized by a constant expression}} expected-note {{reinterpret_cast is not allowed in a constant expression}}
constexpr auto arbitrary = reinterpret_cast<const Record*>(16); // expected-error {{must be initialized by a constant expression}} expected-note {{reinterpret_cast is not allowed in a constant expression}}
int side_effect();
constexpr auto effects = reinterpret_cast<const char*>(&reinterpret_cast<const Record*>((side_effect(), 16))->second) - reinterpret_cast<const char*>(16); // expected-error {{must be initialized by a constant expression}} expected-note {{reinterpret_cast is not allowed in a constant expression}}

// Inherited fields (delta-review follow-up on p33 patch 0010; cl measured
// 2026-11-11, D:\Build\ort-p33-negtests\probe2.cmd MARK A, probe5.cmd MARK G):
// cl evaluates a direct base member at the true object offset, at any base
// depth and through a second non-virtual base. Before the base walk this
// file rejected every inherited shape (fails-before for the walk).
struct Base2 { int first; };
struct Mid2 : Base2 { int mid; };
struct Derived2 : Mid2 { int second[3]; };
struct LeftBase { int left; };
struct RightBase { int right; };
struct Multi2 : LeftBase, RightBase {};
static_assert(OFFSET(Derived2, first) == 0);
static_assert(OFFSET(Derived2, mid) == 4);
static_assert(OFFSET(Derived2, second) == 8);
static_assert(OFFSET(Multi2, right) == 4);
static_assert(OFFSET(Multi2, left) == 0);

// Through a named member of derived type: cl accepts this too and computes
// the true offset there as well (probe4.cmd MARK E/F: OFFSET(OffsetOuter2,
// inner.first) measures 4 on cl - inner sits at 4 after the int prefix - and
// inner.second is 8; both are the true object offsets, so the walk matches
// cl across the whole measured inherited-field surface).
struct OffsetOuter2 { int prefix; Derived2 inner; };
static_assert(OFFSET(OffsetOuter2, inner.first) == 4);
static_assert(OFFSET(OffsetOuter2, inner.second) == 8);

// A reference member is not address-of-able as a field offset: cl rejects
// with C2131 and clang keeps the normal evaluator (probe2.cmd MARK B).
struct WithRef2 { int& r; };
static_assert(OFFSET(WithRef2, r) == 0); // expected-error {{static assertion expression is not an integral constant expression}} expected-note {{reinterpret_cast is not allowed in a constant expression}}

// A virtual-base path is rejected on cl too (C2131, probe5.cmd MARK H);
// the base walk bails on virtual bases.
struct VBase2 { int vb; };
struct WithV2 : virtual VBase2 { int w; };
static_assert(OFFSET(WithV2, vb) == 0); // expected-error {{static assertion expression is not an integral constant expression}} expected-note {{reinterpret_cast is not allowed in a constant expression}}
