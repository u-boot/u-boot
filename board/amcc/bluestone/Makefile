#
# Copyright (c) 2010, Applied Micro Circuits Corporation
# Author: Tirumala R Marri <tmarri@apm.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-y	:= $(BOARD).o
SOBJS	:= init.o

COBJS   := $(COBJS-y)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

all:	$(LIB) $(SOBJS)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $^)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
