#
#  (C) Copyright 2010-2012
#  NVIDIA Corporation <www.nvidia.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

$(shell mkdir -p $(obj)../../nvidia/common)

LIB	= $(obj)lib$(BOARD).o

COBJS-y	:= $(BOARD).o

include ../../nvidia/common/common.mk

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
