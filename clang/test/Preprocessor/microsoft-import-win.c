// A file that is not a type library.
// RUN: %clang_cc1 -E -verify -fms-compatibility -fms-import-output-dir=%t %s
// REQUIRES: system-windows

#import "pp-record.h" // expected-error {{#import of type library 'pp-record.h' failed: cannot load type library}}

#import "pp-record.h" no_namespace, auto_rename // expected-error {{#import of type library 'pp-record.h' failed: cannot load type library}}

// Like cl (C4185/C4186): warn, ignore that attribute and the rest.
#import "pp-record.h" bogus_attribute raw_interfaces_only // expected-warning {{ignoring unknown #import attribute 'bogus_attribute'}} expected-error {{cannot load type library}}
#import "pp-record.h" lcid raw_interfaces_only // expected-warning {{#import attribute 'lcid' requires 1 arguments; ignored}} expected-error {{cannot load type library}}
#import "pp-record.h" rename("a") // expected-warning {{#import attribute 'rename' requires 2 arguments; ignored}} expected-error {{cannot load type library}}
