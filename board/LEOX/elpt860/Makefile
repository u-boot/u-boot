
#######################################################################
#
# Copyright (C) 2000, 2001, 2002, 2003
# The LEOX team <team@leox.org>, http://www.leox.org
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# LEOX.org is about the development of free hardware and software resources
#   for system on chip.
#
# Description: U-Boot port on the LEOX's ELPT860 CPU board
# ~~~~~~~~~~~
#
#######################################################################
#
# SPDX-License-Identifier:	GPL-2.0+
#
#######################################################################

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	= $(BOARD).o flash.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
