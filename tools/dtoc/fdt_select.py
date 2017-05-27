#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

# Bring in the normal fdt library (which relies on libfdt)
import fdt

def FdtScan(fname):
    """Returns a new Fdt object from the implementation we are using"""
    dtb = fdt.Fdt(fname)
    dtb.Scan()
    return dtb
