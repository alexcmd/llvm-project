/*===---- __msvc_eh_predef.h - C++ EH ABI types predeclared for MSVC mode --===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

/* Force-included by the driver with -fms-predeclare-eh-types.
 *
 * Predeclares the C++ exception-handling record types of the Microsoft C++
 * ABI in every C++ translation unit, before any source is read, so that a
 * forced-include header can name them by their ABI tags.
 *
 * ABI layout facts encoded below:
 *  - Field order and sizes follow the ABI. _TypeDescriptor is packed to 8
 *    bytes; the other records are packed to 4 (the #pragma pack regions).
 *  - Pointer fields are pointers on every target, including x64 and ARM64,
 *    where the EH tables emitted for those targets hold 32-bit image-relative
 *    offsets instead of pointers.
 *  - Struct tags use the ABI names (for example _s__ThrowInfo).
 *  - C++ only; the types are not declared in C.
 */

#ifndef __MSVC_EH_PREDEF_H
#define __MSVC_EH_PREDEF_H

#ifdef __cplusplus

#pragma pack(push, 4)

typedef struct _PMD {
  int mdisp; /* member displacement */
  int pdisp; /* vbptr displacement, -1 if none */
  int vdisp; /* displacement within the vbtable */
} _PMD;

typedef void(__cdecl *_PMFN)(void *);

#pragma pack(push, 8)
typedef struct _TypeDescriptor {
  const void *pVFTable; /* type_info vftable */
  void *spare;          /* cached undecorated name */
  char name[];          /* decorated name */
} _TypeDescriptor;
#pragma pack(pop)

typedef const struct _s__CatchableType {
  unsigned int properties;
  _TypeDescriptor *pType;
  _PMD thisDisplacement;
  int sizeOrOffset;
  _PMFN copyFunction;
} _CatchableType;

typedef const struct _s__CatchableTypeArray {
  int nCatchableTypes;
  _CatchableType *arrayOfCatchableTypes[];
} _CatchableTypeArray;

typedef const struct _s__ThrowInfo {
  unsigned int attributes;
  _PMFN pmfnUnwind;
  int(__cdecl *pForwardCompat)(...);
  _CatchableTypeArray *pCatchableTypeArray;
} _ThrowInfo;

#pragma pack(pop)

#endif /* __cplusplus */

#endif /* __MSVC_EH_PREDEF_H */
