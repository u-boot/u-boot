#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
#
# SAMSUNG SMDK6400 board with mDirac3 (ARM1176) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG

# On SMDK6400 we use the 64 MB SDRAM bank at
#
# 0x50000000 to 0x58000000
#
# Linux-Kernel is expected to be at 0x50008000, entry 0x50008000
#
# we load ourselves to 0x57e00000 without MMU
# with MMU, load address is changed to 0xc7e00000
#
# download area is 0x5000c000

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

ifndef CONFIG_NAND_SPL
TEXT_BASE = $(RAM_TEXT)
else
TEXT_BASE = 0
endif

LDSCRIPT := $(SRCTREE)/board/$(BOARDDIR)/u-boot-nand.lds
