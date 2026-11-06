# REQUIRES: x86

# /Brepro with a PDB: the GUID in the image is a hash of the image, not of the
# PDB (which records the absolute output and PDB paths), so an image with a
# relative /pdbaltpath is byte-identical whatever directory it is linked in.
# The PDB carries the same GUID and age as the image.

# RUN: rm -rf %t && mkdir -p %t/a %t/b/sub
# RUN: llvm-mc -filetype=obj -triple=x86_64-windows-msvc %s -o %t/main.obj
# RUN: lld-link /Brepro /debug /pdbaltpath:%%_PDB%% /entry:main /subsystem:console \
# RUN:   /out:%t/a/app.exe %t/main.obj
# RUN: lld-link /Brepro /debug /pdbaltpath:%%_PDB%% /entry:main /subsystem:console \
# RUN:   /out:%t/b/sub/app.exe /pdb:%t/b/sub/app.pdb %t/main.obj
# RUN: cmp %t/a/app.exe %t/b/sub/app.exe

# Linking again into the same directory gives the same image too.
# RUN: cp %t/a/app.exe %t/a/first.exe
# RUN: lld-link /Brepro /debug /pdbaltpath:%%_PDB%% /entry:main /subsystem:console \
# RUN:   /out:%t/a/app.exe %t/main.obj
# RUN: cmp %t/a/app.exe %t/a/first.exe

# RUN: llvm-readobj --coff-debug-directory %t/a/app.exe > %t/ids.txt
# RUN: llvm-pdbutil dump --summary %t/a/app.pdb >> %t/ids.txt
# RUN: llvm-pdbutil dump --summary %t/b/sub/app.pdb >> %t/ids.txt
# RUN: FileCheck %s < %t/ids.txt

# CHECK:      PDBGUID: [[GUID:\{[0-9A-F-]+\}]]
# CHECK-NEXT: PDBAge: 1
# CHECK-NEXT: PDBFileName: app.pdb
# CHECK:      Age: 1
# CHECK-NEXT: GUID: [[GUID]]
# CHECK:      Age: 1
# CHECK-NEXT: GUID: [[GUID]]

  .text
  .globl main
main:
  xorl %eax, %eax
  retq
