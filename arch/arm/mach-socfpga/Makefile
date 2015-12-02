#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2012 Altera Corporation <www.altera.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= misc.o timer.o reset_manager.o system_manager.o clock_manager.o \
	   fpga_manager.o board.o

obj-$(CONFIG_SPL_BUILD) += spl.o freeze_controller.o

# QTS-generated config file wrappers
obj-$(CONFIG_TARGET_SOCFPGA_GEN5)	+= scan_manager.o wrap_pll_config.o
obj-$(CONFIG_SPL_BUILD) += wrap_iocsr_config.o wrap_pinmux_config.o	\
			   wrap_sdram_config.o
CFLAGS_wrap_iocsr_config.o	+= -I$(srctree)/board/$(BOARDDIR)
CFLAGS_wrap_pinmux_config.o	+= -I$(srctree)/board/$(BOARDDIR)
CFLAGS_wrap_pll_config.o	+= -I$(srctree)/board/$(BOARDDIR)
CFLAGS_wrap_sdram_config.o	+= -I$(srctree)/board/$(BOARDDIR)
