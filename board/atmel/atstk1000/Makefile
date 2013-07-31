#
# (C) Copyright 2001-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2005-2006 Atmel Corporation
#
# SPDX-License-Identifier:	GPL-2.0+

include $(TOPDIR)/config.mk

LIB			:= $(obj)lib$(BOARD).o

COBJS-y			+= $(BOARD).o

SRCS			:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS			:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
