#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

CONFIG_STANDALONE_LOAD_ADDR ?= 0xffffffff80200000 \
			       -T $(srctree)/examples/standalone/mips64.lds
