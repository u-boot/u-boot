#
# Copyright (c) Switchfin Org. <dpn@switchfin.org>
#
# Copyright (c) 2005-2008 Analog Device Inc.
#
# (C) Copyright 2001
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# FIX ME
ifneq ($(filter lib lib/lzma lib/zlib, $(obj)),)
ccflags-y := -O2
endif
