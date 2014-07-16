#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= board.o

# Please copy ps7_init.c/h from hw project to this directory
obj-$(CONFIG_SPL_BUILD) += \
		$(if $(wildcard $(srctree)/$(src)/ps7_init.c), ps7_init.o)
