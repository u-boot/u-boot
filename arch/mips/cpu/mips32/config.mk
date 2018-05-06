# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, <wd@denx.de>

CONFIG_STANDALONE_LOAD_ADDR ?= 0x80200000 \
			       -T $(srctree)/examples/standalone/mips.lds
