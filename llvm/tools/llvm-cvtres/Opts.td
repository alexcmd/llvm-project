include "llvm/Option/OptParser.td"

// All the switches can be preceded by either '/' or '-'.

// UE-CVTRES-FOLDDUPS: cvtres 14.51 options and their effect (measured).
def DEFINE : Joined<["/", "-"], "DEFINE:">, HelpText<"Add an absolute external symbol">, MetaVarName<"symbol">;
def FOLDDUPS : Flag<["/", "-"], "FOLDDUPS">, HelpText<"Accepted; cvtres 14.51 folds nothing (duplicates stay CVT1100)">;
def MACHINE : Joined<["/", "-"], "MACHINE:">, HelpText<"Machine architecture">, MetaVarName<"{ARM|ARM64|EBC|IA64|X64|X86}">;
def NOLOGO : Flag<["/", "-"], "NOLOGO">, HelpText<"Accepted (no banner is printed)">;
def OUT : Joined<["/", "-"], "OUT:">, HelpText<"Output file">, MetaVarName<"filename">;
def READONLY : Flag<["/", "-"], "READONLY">, HelpText<"Make the resource data section read-only">;
def VERBOSE : Flag<["/", "-"], "VERBOSE">, HelpText<"Use verbose output">;
def HELP : Flag<["/", "-"], "HELP">, HelpText<"Display available options">;
def H : Flag<["/", "-"], "H">, Alias<HELP>;
def HELP_Q : Flag<["/?", "-?"], "">, Alias<HELP>;

// Extensions.

def TIMESTAMP : Joined<["/", "-"], "TIMESTAMP:">,
    HelpText<"Timestamp for coff header, defaults to current time">;
