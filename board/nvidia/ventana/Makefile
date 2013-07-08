#
#  (C) Copyright 2010,2011
#  NVIDIA Corporation <www.nvidia.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

$(shell mkdir -p $(obj)../seaboard)

LIB	= $(obj)lib$(BOARD).o

COBJS	= ../seaboard/seaboard.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
