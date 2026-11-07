// openrt UE-EXPLOG-SARIF: /experimental:log <path>, joined or as the next
// argument (UnrealBuildTool), is the SARIF log of the diagnostics; the path
// is not an input. A path ending in a separator names a directory, one
// without extension gets .sarif (cl 14.51).

// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /Fo%t.obj \
// RUN:   /experimental:log %t.log.sarif -- %s 2>&1 | FileCheck --check-prefix=SEP %s
// SEP-NOT: cannot specify
// SEP: "-diagnostic-sarif-file" "{{[^"]*}}.log.sarif"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /experimental:logjoined.sarif -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=JOINED %s
// JOINED: "-diagnostic-sarif-file" "joined.sarif"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /experimental:log noext -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NOEXT %s
// NOEXT: "-diagnostic-sarif-file" "noext.sarif"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c -### /experimental:log logdir/ -- %s 2>&1 \
// RUN:   | FileCheck --check-prefix=DIR %s
// DIR: "-diagnostic-sarif-file" "logdir{{/|\\\\}}cl-experimental-log.sarif"

// The log itself: one result per warning, cl warning id as ruleId.
// RUN: rm -f %t.sarif
// RUN: %clang_cl --target=x86_64-pc-windows-msvc /c /W4 /Fo%t.obj /experimental:log %t.sarif -- %s
// RUN: FileCheck --check-prefix=LOG %s < %t.sarif
// LOG: "version": "2.1.0",
// LOG: "runs": [
// LOG: "results": [
// LOG: "ruleId": "C4101",
// LOG: "text": "unused variable 'x'"
// LOG: "uri": "file:///
// LOG: "startLine": [[@LINE+3]],
// LOG: "startColumn": 7
int f(void) {
  int x;
  return 0;
}
