#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-$(CONFIG_MACH_DAVINCI_DA830_EVM)	+= da830evm.o
COBJS-$(CONFIG_MACH_DAVINCI_DA850_EVM)	+= da850evm.o
COBJS-$(CONFIG_MACH_DAVINCI_HAWK)	+= hawkboard.o

COBJS   := $(COBJS-y)

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################
# This is for $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
