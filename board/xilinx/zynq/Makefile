#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= board.o

ifneq ($(wildcard ps7_init.c),)
ifneq ($(wildcard ps7_init.h),)
obj-$(CONFIG_SPL_BUILD)	+= ps7_init.o
ifeq ($(CONFIG_SPL_BUILD),y)
$(shell touch xil_io.h)
endif
endif
endif
