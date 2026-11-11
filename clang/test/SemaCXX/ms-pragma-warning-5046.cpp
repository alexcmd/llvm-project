// openrt UE-C5046: an inline body uses a member declared in this TU whose
// definition lives in another file. cl reports C5046 (level 2: silent at
// /W0../W1, reported at /W2../W4); clang reports -Wundefined-internal.
// The id maps, so /wd5046 and #pragma warning(disable: 5046) reach it.

// cl's default level (no /W flag): silent, as cl is. Silent runs pipe through
// `count 0` (FileCheck rejects empty input).
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fsyntax-only -- %s 2>&1 | count 0
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W1 -fsyntax-only -- %s 2>&1 | count 0

// From /W2, as cl reports the C5046.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W2 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=ON %s
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 -fsyntax-only -- %s 2>&1 | FileCheck --check-prefix=ON %s
// ON-COUNT-2: warning: {{.*}}has internal linkage but is not defined
// ON-NOT: warning:

// /wd5046 silences it, as cl's /wd does.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /W4 /wd5046 -fsyntax-only -- %s 2>&1 | count 0

namespace {
struct Used {
  Used(); // declared, never defined here
};
struct UseUsed {
  Used f;
  UseUsed() = default; // references Used::Used
};

#pragma warning(push)
#pragma warning(disable : 5046)
struct Suppressed {
  Suppressed(); // declared, never defined here
};
struct UseSuppressed {
  Suppressed f;
  UseSuppressed() = default;
};
#pragma warning(pop)

struct Restored {
  Restored(); // declared, never defined here
};
struct UseRestored {
  Restored f;
  UseRestored() = default; // references Restored::Restored
};

// The defaulted constructors above are only materialized when odr-used; a
// never-called function that constructs each type forces that materialization
// (without these, no -Wundefined-internal fires and the whole test is inert).
void touch() {
  UseUsed a;
  UseSuppressed b;
  UseRestored c;
  (void)a; (void)b; (void)c;
}
} // namespace
