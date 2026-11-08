// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify -Wshorten-64-to-32 %s

// openrt UE-MASK-CONSTEXPR (r27, p27 regression): u32 - (inline constexpr
// u64 = 1) passed as int: width max(32, 1) = 32, silent in cl 14.51.
// expected-no-diagnostics

typedef unsigned long long SIZE_T_;
inline constexpr SIZE_T_ SkipCount = 1;
void consume(int index, void *pc);
void log_frames(void **frames, unsigned depth) {
  for (unsigned i = SkipCount; i < depth; ++i)
    consume(i - SkipCount, frames[i]);
}
