#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import fdt_fallback

# Bring in either the normal fdt library (which relies on libfdt) or the
# fallback one (which uses fdtget and is slower). Both provide the same
# interface for this file to use.
try:
    import fdt_normal
    have_libfdt = True
except ImportError:
    have_libfdt = False

force_fallback = False

def FdtScan(fname, _force_fallback=False):
    """Returns a new Fdt object from the implementation we are using"""
    if have_libfdt and not force_fallback and not _force_fallback:
        dtb = fdt_normal.FdtNormal(fname)
    else:
        dtb = fdt_fallback.FdtFallback(fname)
    dtb.Scan()
    return dtb

def UseFallback(fallback):
    global force_fallback

    old_val = force_fallback
    force_fallback = fallback
    return old_val
