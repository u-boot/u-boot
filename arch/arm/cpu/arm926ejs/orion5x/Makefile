#
# Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
#
# Based on original Kirkwood support which is
# (C) Copyright 2009
# Marvell Semiconductor <www.marvell.com>
# Written-by: Prafulla Wadaskar <prafulla@marvell.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-y	= cpu.o
COBJS-y	+= dram.o
COBJS-y	+= timer.o

ifndef CONFIG_SKIP_LOWLEVEL_INIT
SOBJS	:= lowlevel_init.o
endif

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
