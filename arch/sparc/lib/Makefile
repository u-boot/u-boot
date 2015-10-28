#
# (C) Copyright 2000-2015
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y = cache.o interrupts.o time.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o

ifndef CONFIG_SYS_GENERIC_BOARD
obj-y += board.o
endif
