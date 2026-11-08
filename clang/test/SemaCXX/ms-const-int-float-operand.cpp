// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -Wimplicit-const-int-float-conversion -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -std=c++20 \
// RUN:   -fsyntax-only -Wimplicit-const-int-float-conversion -verify=std %s

// openrt UE-CONST-INT-FLOAT (r32): cl 14.51 reports an int constant that does
// not fit the float as C4305 (level 1) only for initialization, assignment,
// argument and return. An operand of a binary operator (comparison, compound
// assignment) is C5219 (off by default); a ?: arm is silent (/Wall).

void tf(float);
const int M = 2147483647;

unsigned clamp_u(float v) { return (unsigned)(v > M ? M : v); } // std-warning 2 {{changes value from 2147483647 to 2147483648}}
float cmp(float v) { return v > M ? v : 0.f; }                  // std-warning {{changes value}}
float add(float v) { return v + 16777217; }                     // std-warning {{changes value}}
float cadd(float f) { f += M; return f; }                       // std-warning {{changes value}}
float arm(bool b) { float r = b ? M : 1.0f; return r; }         // std-warning {{changes value}}
double cmp64(double d) { const long long L = 9223372036854775807LL; return d < L ? d : L; } // std-warning 2 {{changes value}}

float ret() { return M; }                  // ms-warning {{changes value}} std-warning {{changes value}}
float init() { float r = M; return r; }    // ms-warning {{changes value}} std-warning {{changes value}}
float asg(float f) { f = M; return f; }    // ms-warning {{changes value}} std-warning {{changes value}}
void arg() { tf(M); }                      // ms-warning {{changes value}} std-warning {{changes value}}
float neg() { return -M; }                 // ms-warning {{changes value}} std-warning {{changes value}}
