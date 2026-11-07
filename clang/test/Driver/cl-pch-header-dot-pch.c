// openrt UE-PCH-GCHPROBE: no gcc-style PCH probe in cl mode. UnrealBuildTool
// names the PCH <header>.pch next to the header and passes
// /FI<h> /Yu<h> /Fp<h>.pch; the /FI must stay an -include of the through
// header, with a single -include-pch.

// RUN: rm -rf %t && mkdir -p %t
// RUN: echo "#pragma once" > %t/P.h
// RUN: echo "not a pch" > %t/P.h.pch
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /FI%t/P.h /Yu%t/P.h \
// RUN:   /Fp%t/P.h.pch -- %s 2>&1 | FileCheck %s
// CHECK: "-include-pch" "{{[^"]*}}P.h.pch"
// CHECK-NOT: "-include-pch"
// CHECK: "-include" "{{[^"]*}}P.h"
// CHECK-NOT: "-include-pch"
