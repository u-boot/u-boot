#
# Copyright (C) 2007,2008 Nobuhiro Iwamatsu
#
# board/r7780mp/Makefile
#
# SPDX-License-Identifier:	GPL-2.0+

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= r7780mp.o
SOBJS	:= lowlevel_init.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
