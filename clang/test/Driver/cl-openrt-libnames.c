// LIBNAME-OPENRT: the default libs the driver records come from one table of
// openrt names (OpenRTLibNames.h): /MD /MDd /MT /MTd pick the runtime
// flavour, plus the old-names library; /Zl drops both. The clang driver on
// the MSVC target passes the static flavour to the linker. /openmp excludes no
// other OpenMP runtime by name. The libmap named by the manifest.json of an
// openrt layout around the driver may map the names onto other files (here a
// data-only layout, "toolchain": false, which is not taken as the toolchain
// root).

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /MD -- %s 2>&1 | FileCheck --check-prefix=MD %s
// MD: "--dependent-lib=openruntime_md" "--dependent-lib=openoldnames"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /MDd -- %s 2>&1 | FileCheck --check-prefix=MDD %s
// MDD: "--dependent-lib=openruntime_mdd" "--dependent-lib=openoldnames"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /MT -- %s 2>&1 | FileCheck --check-prefix=MT %s
// MT: "--dependent-lib=openruntime_mt" "--dependent-lib=openoldnames"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /MTd -- %s 2>&1 | FileCheck --check-prefix=MTD %s
// MTD: "--dependent-lib=openruntime_mtd" "--dependent-lib=openoldnames"
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -### -c /MD /Zl -- %s 2>&1 | FileCheck --check-prefix=ZL %s
// ZL-NOT: "--dependent-lib=open

// RUN: %clang --target=x86_64-pc-windows-msvc -fuse-ld=lld -### %s 2>&1 | FileCheck --check-prefix=LINK %s
// LINK: "-defaultlib:openruntime_mt" "-defaultlib:openoldnames"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -fuse-ld=lld /openmp -### -- %s 2>&1 | FileCheck --check-prefix=OMP %s
// OMP-NOT: vcomp
// OMP: "-defaultlib:libomp.lib"
// OMP-NOT: vcomp

// Data-only layout: names mapped, root not used as toolchain.
// RUN: rm -rf %t && mkdir -p %t/dev/bin %t/dev/lib
// RUN: echo '{"toolchain": false, "libmap": "lib/openrt-libmap.txt"}' > %t/dev/manifest.json
// RUN: printf '# test data\nopenruntime_md\tcrt_dll\nOpenOldNames.lib other_old.lib\n' > %t/dev/lib/openrt-libmap.txt
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/dev/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -c /MD -- %s 2>&1 | FileCheck --check-prefix=MAP %s
// MAP: "--dependent-lib=crt_dll" "--dependent-lib=other_old.lib"
// RUN: env -u INCLUDE -u EXTERNAL_INCLUDE -u LIB -u VCToolsInstallDir -u VCINSTALLDIR \
// RUN:   %clang_cl -ccc-install-dir %t/dev/bin --target=x86_64-pc-windows-msvc \
// RUN:   -### -c /MT -- %s 2>&1 | FileCheck --check-prefix=MAPMT %s
// MAPMT-NOT: "-internal-isystem" "{{[^"]*}}dev{{[/\\]+}}include"
// MAPMT: "--dependent-lib=openruntime_mt" "--dependent-lib=other_old.lib"
// MAPMT-NOT: "-internal-isystem" "{{[^"]*}}dev{{[/\\]+}}include"

// The fork source has no other default-lib names for these.
// RUN: not grep -E '"(--dependent-lib=|-defaultlib:)?(msvcrtd?|libcmtd?|oldnames|vcompd?)(\.lib)?"' \
// RUN:   %S/../../lib/Driver/ToolChains/Clang.cpp %S/../../lib/Driver/ToolChains/MSVC.cpp \
// RUN:   %S/../../lib/Driver/ToolChains/Flang.cpp %S/../../lib/Driver/ToolChains/OpenRTLibNames.h

int main(void) { return 0; }
