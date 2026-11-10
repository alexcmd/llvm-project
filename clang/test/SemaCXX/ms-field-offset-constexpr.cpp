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
