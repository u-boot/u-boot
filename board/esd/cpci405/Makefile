#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../common)
endif

obj-y	= cpci405.o flash.o ../common/misc.o ../common/auto_update.o
obj-y	+= ../common/cmd_loadpci.o
