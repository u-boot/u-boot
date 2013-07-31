#
# Copyright (c) 2005-2008 Analog Device Inc.
#
# (C) Copyright 2001
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

CFLAGS_lib += -O2
CFLAGS_lib/lzma += -O2
CFLAGS_lib/zlib += -O2

# Set some default LDR flags based on boot mode.
LDR_FLAGS-BFIN_BOOT_PARA := --bits 16
