#
# Copyright 2012 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-$(CONFIG_T4240QDS) += t4240qds.o
COBJS-$(CONFIG_T4240EMU) += t4240emu.o
COBJS-y	+= ddr.o
COBJS-$(CONFIG_T4240QDS)+= eth.o
COBJS-$(CONFIG_PCI)	+= pci.o
COBJS-y	+= law.o
COBJS-y	+= tlb.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS))

clean:
	rm -f $(OBJS) $(SOBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
