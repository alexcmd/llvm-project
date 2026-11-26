// CL-OPT-INCOMPATIBLE: cl 19.44 (observed) stops with D8016 (exit 2) at the
// first pair of options that cannot go together and names them in command
// line order (/arch:<x> before /kernel); one pair only.

// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /RTC1 /O2 -- %s 2>&1 | FileCheck --check-prefix=RTC %s
// RTC: error: D8016 : '/RTC1' and '/O2' command-line options are incompatible
// RTC-NOT: D8016
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /O2 /RTCs /Gd /Gz -- %s 2>&1 | FileCheck --check-prefix=ORTC %s
// ORTC: error: D8016 : '/O2' and '/RTCs' command-line options are incompatible
// ORTC-NOT: D8016
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /RTCu /Ox -- %s 2>&1 | FileCheck --check-prefix=RTCX %s
// RTCX: D8016 : '/RTCu' and '/Ox' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /ZI /GL -- %s 2>&1 | FileCheck --check-prefix=GL %s
// GL: D8016 : '/ZI' and '/GL' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Gz /Gd /Gr -- %s 2>&1 | FileCheck --check-prefix=CC %s
// CC: D8016 : '/Gz' and '/Gd' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /utf-8 /source-charset:utf-8 -- %s 2>&1 | FileCheck --check-prefix=UTF8 %s
// UTF8: D8016 : '/utf-8' and '/source-charset:utf-8' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /execution-charset:utf-8 /utf-8 -- %s 2>&1 | FileCheck --check-prefix=UTF8E %s
// UTF8E: D8016 : '/execution-charset:utf-8' and '/utf-8' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /std:c11 /std:c++17 -- %s 2>&1 | FileCheck --check-prefix=STD %s
// STD: D8016 : '/std:c11' and '/std:c++17' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /vmb /vmg -- %s 2>&1 | FileCheck --check-prefix=VM %s
// VM: D8016 : '/vmb' and '/vmg' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /vms /vmv -- %s 2>&1 | FileCheck --check-prefix=VMV %s
// VMV: D8016 : '/vms' and '/vmv' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /GR /kernel -- %s 2>&1 | FileCheck --check-prefix=KGR %s
// KGR: D8016 : '/GR' and '/kernel' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /kernel /arch:AVX2 -- %s 2>&1 | FileCheck --check-prefix=KARCH %s
// KARCH: D8016 : '/arch:AVX2' and '/kernel' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /fp:except /fp:fast -- %s 2>&1 | FileCheck --check-prefix=FP %s
// FP: D8016 : '/fp:except' and '/fp:fast' command-line options are incompatible
// RUN: not %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /Za /Ze -- %s 2>&1 | FileCheck --check-prefix=ZA %s
// ZA: D8016 : '/Za' and '/Ze' command-line options are incompatible


// Not incompatible: /RTC1 with /Od /Os /Ot; /kernel /arch:SSE2 on x64;
// /fp:fast /fp:except-; /source-charset with /execution-charset.
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /nologo /c /RTC1 /Od /Os /Ot /kernel /arch:SSE2 /fp:fast /fp:except- /source-charset:utf-8 /execution-charset:utf-8 -### -- %s 2>&1 | FileCheck --check-prefix=OK %s
// OK-NOT: D8016
