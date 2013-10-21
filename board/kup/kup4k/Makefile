#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../common)
endif

obj-y	= kup4k.o ../common/flash.o ../common/kup.o ../common/load_sernum_ethaddr.o ../common/pcmcia.o
