// A file that is not a type library.
// RUN: %clang_cc1 -E -verify -fms-compatibility -fms-import-output-dir=%t %s
// REQUIRES: system-windows

#import "pp-record.h" // expected-error {{#import of type library 'pp-record.h' failed: cannot load type library}}

#import "pp-record.h" no_namespace, auto_rename // expected-error {{#import of type library 'pp-record.h' failed: cannot load type library}}

#import "pp-record.h" bogus_attribute // expected-error {{unknown #import attribute 'bogus_attribute'}}
