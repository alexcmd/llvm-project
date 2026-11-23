# Runs a command with LIB and the Visual Studio environment removed, the same on
# any host: lit's `env -u` (msys env on the openrt host) does not remove the
# upper-case Windows names (VCTOOLSINSTALLDIR), so tests scrub here instead.
# Usage: scrub-env.py [NAME=VALUE ...] -- command [args...]
# NAME=VALUE pairs are added after the scrub (e.g. an empty LIB=).
import os
import subprocess
import sys

VS_VARS = {'LIB', 'LIBPATH', 'INCLUDE', 'EXTERNAL_INCLUDE', 'VCTOOLSINSTALLDIR',
           'VCTOOLSVERSION', 'VCINSTALLDIR', 'VSINSTALLDIR', 'WINDOWSSDKDIR',
           'WINDOWSSDKVERSION', 'WINDOWSSDKLIBVERSION', 'LINK', '_LINK_'}
args = sys.argv[1:]
sep = args.index('--')
env = {k: v for k, v in os.environ.items() if k.upper() not in VS_VARS}
for kv in args[:sep]:
    k, v = kv.split('=', 1)
    env[k] = v
sys.exit(subprocess.call(args[sep + 1:], env=env))
