#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
# Copyright (C) 2008 Renesas Solutions Corp.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

SOBJS	= start.o
COBJS	= cpu.o interrupts.o watchdog.o

SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS    := $(addprefix $(obj),$(COBJS))
SOBJS   := $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
