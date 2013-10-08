#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2012 Altera Corporation <www.altera.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#


include $(TOPDIR)/config.mk

LIB	=  $(obj)lib$(SOC).o

SOBJS	:= lowlevel_init.o
COBJS-y	:= misc.o timer.o reset_manager.o system_manager.o
COBJS-$(CONFIG_SPL_BUILD) += spl.o

COBJS	:= $(COBJS-y)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
