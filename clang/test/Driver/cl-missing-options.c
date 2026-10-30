// Documented cl.exe switches that clang-cl lacked are accepted as no-ops
// (tasks/compat/10). /PD warns D9007 without /Zc:preprocessor, as cl does.
// /templateDepth:N maps to -ftemplate-depth=N.

// RUN: %clang_cl /c /Gu /PH /feature:APX /fpcvt:IA /fpcvt:BC /jumptablerdata \
// RUN:   /limitTemplateNotes:5 /presetPadding /volatileMetadata \
// RUN:   /volatileMetadata- /sourcelink:x.json /templateDepth:1000 -### \
// RUN:   -- %s 2>&1 | FileCheck %s --check-prefix=NOOP
// NOOP-NOT: warning:
// NOOP-NOT: error:
// NOOP: "-ftemplate-depth=1000"

// RUN: %clang_cl /c /PD -### -- %s 2>&1 | FileCheck %s --check-prefix=PD
// PD: warning: D9007 : '/PD' requires '/Zc:preprocessor'; option ignored

// RUN: %clang_cl /c /PD /Zc:preprocessor -### -- %s 2>&1 | FileCheck %s --check-prefix=PDZC
// PDZC-NOT: D9007
// PDZC-NOT: warning

// Undocumented cl.exe toggles are accepted silently.
// RUN: %clang_cl /c /Zf /WL- /typedil /presetPointers- /presetScalars \
// RUN:   /allowSuper -### -- %s 2>&1 | FileCheck %s --check-prefix=NOOP-HIDDEN
// NOOP-HIDDEN-NOT: warning:
// NOOP-HIDDEN-NOT: error:

// Other spellings are not cl switches: unknown /options stay D9002 (ignored).
// RUN: %clang_cl /c /feature:FOO -### -- %s 2>&1 | FileCheck %s --check-prefix=UNK
// UNK: warning: D9002 : ignoring unknown option '/feature:FOO'
