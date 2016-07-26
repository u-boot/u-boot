#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

# Bring in either the normal fdt library (which relies on libfdt) or the
# fallback one (which uses fdtget and is slower). Both provide the same
# interface for this file to use.
try:
    import fdt
    have_libfdt = True
except ImportError:
    have_libfdt = False
    import fdt_fallback as fdt

def FdtScan(fname):
    """Returns a new Fdt object from the implementation we are using"""
    dtb = fdt.Fdt(fname)
    dtb.Scan()
    return dtb
