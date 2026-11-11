// A runtime can include stddef.h while its corecrt dependencies are incomplete.
#error stddef.h must not include corecrt.h to obtain SAL annotations
