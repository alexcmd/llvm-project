/*===---- __msvc_eh_predef.h - C++ EH types pre-injected by cl.exe ---------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

/* Force-included by the driver with -fms-predeclare-eh-types.
 *
 * cl.exe declares the following types in every C++ translation unit before
 * any source is read. MSVC's <ehdata_forceinclude.h> uses them (e.g. the
 * _CxxThrowException prototype takes a _ThrowInfo *) and maps only the
 * unprefixed names (ThrowInfo, CatchableType, ...) to its own definitions.
 *
 * Unlike the unprefixed runtime records, the pre-injected types use pointer
 * fields on every target, including x64/ARM64 where the emitted EH data holds
 * 32-bit image-relative offsets. Sizes, offsets, field types and the mangled
 * tag names below match cl.exe (19.44, x86 and x64). They are not declared in
 * C, as with cl.exe. */

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
