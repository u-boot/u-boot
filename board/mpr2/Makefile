#
# Copyright (C) 2007
# Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
#
# Copyright (C) 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
# Copyright (C) 2007
# Kenati Technologies, Inc.
#
# (C) Copyright 2008
# Mark Jonas <mark.jonas@de.bosch.com>
#
# board/mpr2/Makefile
#
# SPDX-License-Identifier:	GPL-2.0+

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= mpr2.o
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
