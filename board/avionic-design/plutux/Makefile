#
#  (C) Copyright 2010,2011
#  NVIDIA Corporation <www.nvidia.com>
#  (C) Copyright 2011,2012
#  Avionic Design GmbH <www.avionic-design.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

$(shell mkdir -p $(obj)../common $(obj)../../nvidia/common)

LIB	= $(obj)lib$(BOARD).o

COBJS-y	:= ../common/tamonten.o

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
