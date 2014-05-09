#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SYS_GENERIC_BOARD
obj-y	+= board.o
endif
obj-y	+= io.o

obj-$(CONFIG_CMD_BOOTM) += bootm.o

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += ashldi3.o ashrdi3.o lshrdi3.o
