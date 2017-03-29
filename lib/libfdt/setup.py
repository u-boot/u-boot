#!/usr/bin/env python

"""
setup.py file for SWIG libfdt
"""

from distutils.core import setup, Extension
import os
import sys

# Don't cross-compile - always use the host compiler.
del os.environ['CROSS_COMPILE']
del os.environ['CC']

progname = sys.argv[0]
cflags = sys.argv[1]
files = sys.argv[2:]

if cflags:
    cflags = [flag for flag in cflags.split(' ') if flag]
else:
    cflags = None

libfdt_module = Extension(
    '_libfdt',
    sources = files,
    extra_compile_args =  cflags
)

sys.argv = [progname, '--quiet', 'build_ext', '--inplace', '--force']

setup (name = 'libfdt',
       version = '0.1',
       author      = "SWIG Docs",
       description = """Simple swig libfdt from docs""",
       ext_modules = [libfdt_module],
       py_modules = ["libfdt"],
       )
