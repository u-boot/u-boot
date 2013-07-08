#
# Copyright (C) 2011 Samsung Electronics
# Heungjun Kim <riverful.kim@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-y	+= trats.o

SRCS    := $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
