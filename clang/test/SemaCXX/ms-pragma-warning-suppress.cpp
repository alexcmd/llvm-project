// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify %s

// openrt UE-getenv: cl's "#pragma warning(suppress : n)" turns the mapped
// warning off on the single line after the pragma (every occurrence on that
// line) and the prior state resumes on the line after that. Measured on cl
// 14.50. Before this patch clang parsed the suppress clause and ignored it, so
// the r2_getenv repro (D:\Build\warn-r5\r2_getenv.cpp) still printed the C4996
// equivalent on the pragma's next line.

[[deprecated]] void callMe() {} // expected-note 9 {{marked 'deprecated' attribute here}}

void next_line_only() {
#pragma warning(suppress : 4996)
  callMe();
  callMe(); // expected-warning {{'callMe' is deprecated}}
}

void all_occurrences_on_the_line() {
#pragma warning(suppress : 4996)
  callMe(); callMe();
}

void window_does_not_skip_lines() {
#pragma warning(suppress : 4996)

  callMe(); // expected-warning {{'callMe' is deprecated}}
}

void pop_takes_the_window_line() {
  // cl: the pop sits on the window line and the next line warns normally.
#pragma warning(push)
#pragma warning(suppress : 4996)
#pragma warning(pop)
  callMe(); // expected-warning {{'callMe' is deprecated}}
}

void disabled_then_suppressed_then_popped() {
  // p4 shape: the disable is gone after the pop; the suppress does not reach
  // past its window line.
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(suppress : 4996)
#pragma warning(pop)
  callMe(); // expected-warning {{'callMe' is deprecated}}
}

void two_suppress_lines() {
  // Each suppress clause gets its own window: the second line's window covers
  // the first call, the first pragma's window (the second pragma's line)
  // expires unused.
#pragma warning(suppress : 4996)
#pragma warning(suppress : 4996)
  callMe();
  callMe(); // expected-warning {{'callMe' is deprecated}}
}

void number_list(int i, unsigned u) {
#pragma warning(suppress : 4996 4018)
  bool a = (callMe(), i < u); (void)a;
  bool b = (callMe(), i < u); (void)b; // expected-warning {{'callMe' is deprecated}} expected-warning {{comparison of integers of different signs}}
}

void unrelated_warning_still_shown(int i, unsigned u) {
  // Only 4996 is suppressed; the sign-compare on the window line is not.
#pragma warning(suppress : 4996)
  bool a = (callMe(), i < u); (void)a; // expected-warning {{comparison of integers of different signs}}
}

void broken_clause_is_dropped() {
  // cl reports C4081 here and suppresses nothing; clang warns its own syntax
  // warning and likewise arms nothing (measured on cl 14.50).
#pragma warning(suppress : 4996, 4018) // expected-warning {{#pragma warning expected ')'}}
  callMe(); // expected-warning {{'callMe' is deprecated}}
}

void unknown_number_is_silent() {
  // cl says C4619 for an unknown number. Clang's warning pragmas stay silent
  // about numbers the wd table does not map (as they already do for disable),
  // and there is nothing to suppress.
#pragma warning(suppress : 99999)
  callMe(); // expected-warning {{'callMe' is deprecated}}
}

void trailing_semicolon_keeps_the_clause() {
  // The suppress clause ended cleanly at the ';' so its window is armed, even
  // though ')' cannot parse as the next clause (cl keeps it too, C4615).
#pragma warning(suppress : 4996 ;) // expected-warning {{#pragma warning expected 'push', 'pop', 'default', 'disable', 'error', 'once', 'suppress', 1, 2, 3, or 4}}
  callMe();
}

void suppress_then_disable(int i, unsigned u) {
#pragma warning(suppress : 4996 ; disable : 4018)
  bool a = (callMe(), i < u); (void)a;
  bool b = (callMe(), i < u); (void)b; // expected-warning {{'callMe' is deprecated}}
}
