// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fms-extensions -fms-compatibility \
// RUN:   -std=c++20 -fsyntax-only -verify=ms %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -std=c++20 -fsyntax-only -verify=std %s

// openrt UE-NSDMI-DEFAULT-ARG (r34): a nested struct with default member
// initializers, default-constructed in a default argument of the enclosing
// class's member functions. cl accepts it; so does MSVC compatibility.
// ms-no-diagnostics

struct FText {};
template <class T> struct TArray { T *p = nullptr; };
struct UStaticMesh {
  struct FBuildParameters { // std-error {{default member initializer for 'bInSilent' needed within definition of enclosing class 'UStaticMesh' outside of member functions}} std-error {{default member initializer for 'OutErrors' needed}}
    bool bInSilent = false; // std-note {{default member initializer declared here}}
    TArray<FText> *OutErrors = nullptr; // std-note {{default member initializer declared here}}
  };
  void Build(const FBuildParameters &P = FBuildParameters()); // std-note {{in evaluation of exception specification}}
};
void f(UStaticMesh &m) { m.Build(); }

// The exception specification is computed at its next use, once parsed.
static_assert(noexcept(UStaticMesh::FBuildParameters()));
