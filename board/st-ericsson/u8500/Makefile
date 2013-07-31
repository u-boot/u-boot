#
# Copyright (C) ST-Ericsson SA 2009
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

CFLAGS += -D__RELEASE -D__STN_8500
LIB	= $(obj)lib$(BOARD).o

COBJS	:= u8500_href.o gpio.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB): $(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
