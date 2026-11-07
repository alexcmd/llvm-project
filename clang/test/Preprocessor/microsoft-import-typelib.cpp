// #import of a type library writes <name>.tlh (+ .tli) into the object file
// directory, laid out as cl 14.51 does, and includes the .tlh. stdole2.tlb
// ships with every Windows.
// REQUIRES: system-windows

// RUN: rm -rf %t && mkdir -p %t/raw %t/full
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -E -fms-extensions \
// RUN:   -fms-compatibility -isystem %S/Inputs/ms-import-typelib \
// RUN:   -fms-import-output-dir=%t/raw -DRAW %s | FileCheck %s --check-prefix=PP
// RUN: FileCheck %s --check-prefix=RAW --input-file=%t/raw/stdole2.tlh
// RUN: not ls %t/raw/stdole2.tli

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -E -fms-extensions \
// RUN:   -fms-compatibility -isystem %S/Inputs/ms-import-typelib \
// RUN:   -fms-import-output-dir=%t/full %s | FileCheck %s --check-prefix=PPFULL
// RUN: FileCheck %s --check-prefix=FULL --input-file=%t/full/stdole2.tlh
// RUN: FileCheck %s --check-prefix=TLI --input-file=%t/full/stdole2.tli

// Up to date (stamp equals the type library's): not rewritten.
// RUN: cp %t/full/stdole2.tlh %t/before.tlh
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -E -fms-extensions \
// RUN:   -fms-compatibility -isystem %S/Inputs/ms-import-typelib \
// RUN:   -fms-import-output-dir=%t/full %s -o /dev/null
// RUN: diff %t/before.tlh %t/full/stdole2.tlh

#ifdef RAW
#import "C:/Windows/System32/stdole2.tlb" raw_interfaces_only named_guids
#else
#import <C:/Windows/System32/stdole2.tlb>
#endif

// PP: namespace stdole {
// PP: {{^}}Font : IDispatch
// PP-NEXT: {};

// RAW: // C++ source equivalent of Win32 type library C:/Windows/System32/stdole2.tlb
// RAW-NEXT: // compiler-generated file - DO NOT EDIT!
// RAW-EMPTY:
// RAW-NEXT: #pragma once
// RAW-NEXT: #pragma pack(push, 8)
// RAW-EMPTY:
// RAW-NEXT: #include <comdef.h>
// RAW-EMPTY:
// RAW-NEXT: namespace stdole {
// RAW-EMPTY:
// RAW-NEXT: //
// RAW-NEXT: // Forward references and typedefs
// RAW-NEXT: //
// RAW-EMPTY:
// RAW-NEXT: struct __declspec(uuid("00020430-0000-0000-c000-000000000046"))
// RAW-NEXT: /* LIBID */ __stdole;
// RAW-NEXT: struct GUID;
// RAW-NOT: IUnknown
// RAW: enum OLE_TRISTATE : int;
// RAW-NEXT: struct __declspec(uuid("bef6e003-a874-101a-8bba-00aa00300cab"))
// RAW-NEXT: /* dispinterface */ Font;
// RAW-NEXT: struct /* coclass */ StdFont;
// RAW: typedef unsigned long OLE_COLOR;
// RAW: typedef struct Font IFontDisp;
// RAW: _COM_SMARTPTR_TYPEDEF(Font, __uuidof(Font));
// RAW: #pragma pack(push, 4)
// RAW-EMPTY:
// RAW-NEXT: struct GUID
// RAW-NEXT: {
// RAW-NEXT:     unsigned long Data1;
// RAW-NEXT:     unsigned short Data2;
// RAW-NEXT:     unsigned short Data3;
// RAW-NEXT:     unsigned char Data4[8];
// RAW-NEXT: };
// RAW: struct __declspec(uuid("0be35203-8f91-11ce-9de3-00aa004bb851"))
// RAW-NEXT: StdFont;
// RAW-NEXT:     // [ default ] dispinterface Font
// RAW-NEXT:     // [ default, source ] dispinterface FontEvents
// RAW-NEXT:     // interface IFont
// RAW:     HRESULT __stdcall LoadPicture(
// RAW-NEXT:         VARIANT filename,
// RAW: // Named GUID constants initializations
// RAW: extern "C" const GUID __declspec(selectany) LIBID_stdole =
// RAW-NEXT:     {0x00020430,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
// RAW: } // namespace stdole
// RAW-EMPTY:
// RAW-NEXT: #pragma pack(pop)

// PPFULL: inline _bstr_t Font::GetName ( ) {
// FULL: // Type library items
// FULL: struct __declspec(uuid("bef6e003-a874-101a-8bba-00aa00300cab"))
// FULL-NEXT: Font : IDispatch
// FULL-NEXT: {
// FULL-NEXT:     //
// FULL-NEXT:     // Property data
// FULL-NEXT:     //
// FULL-EMPTY:
// FULL-NEXT:     __declspec(property(get=GetName,put=PutName))
// FULL-NEXT:     _bstr_t Name;
// FULL:     // Properties:
// FULL-NEXT:     _bstr_t GetName ( );
// FULL-NEXT:     void PutName ( _bstr_t _val );
// FULL:     // Method 'Render' not emitted because argument 'prcWBounds' has invalid type: void *
// FULL: // Wrapper method implementations
// FULL: #include "{{.*}}stdole2.tli"

// TLI: // Wrapper implementations for Win32 type library C:/Windows/System32/stdole2.tlb
// TLI: #pragma once
// TLI: // dispinterface Font wrapper method implementations
// TLI: inline void Font::PutName ( _bstr_t _val ) {
// TLI-NEXT:     _com_dispatch_propput(this, 0x0, VT_BSTR, (BSTR)_val);
// TLI-NEXT: }
// TLI: inline HRESULT FontEvents::FontChanged ( _bstr_t PropertyName ) {
// TLI-NEXT:     return _com_dispatch_method(this, 0x9, DISPATCH_METHOD, VT_EMPTY, NULL,{{ }}
// TLI-NEXT:         L"\x0008", (BSTR)PropertyName);
