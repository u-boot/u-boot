#
#  (C) Copyright 2012 Lucas Stach
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

$(shell mkdir -p $(obj)../../nvidia/common)
$(shell mkdir -p $(obj)../colibri_t20-common)

LIB	= $(obj)lib$(BOARD).o

COBJS	:= ../../nvidia/common/board.o
COBJS	+= ../colibri_t20-common/colibri_t20-common.o
COBJS	+= $(BOARD).o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
