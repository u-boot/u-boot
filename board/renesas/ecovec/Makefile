#
# Copyright (C) 2011 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
# Copyright (C) 2011 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
#
# SPDX-License-Identifier:	GPL-2.0+


include $(TOPDIR)/config.mk

LIB = $(obj)lib$(BOARD).o

COBJS   := ecovec.o
SOBJS   := lowlevel_init.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB): $(OBJS) $(SOBJS)
		$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
