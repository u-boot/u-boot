#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2004
# ARM Ltd.
# Philippe Robin, <philippe.robin@arm.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

SOBJS-y	:= lowlevel_init.o

COBJS-y	:= integrator.o
COBJS-$(CONFIG_PCI) += pci.o
COBJS-y += timer.o

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
COBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS-y))

$(LIB):	$(obj).depend $(COBJS) $(SOBJS)
	$(call cmd_link_o_target, $(COBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
