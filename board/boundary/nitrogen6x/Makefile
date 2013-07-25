#
# Copyright (C) 2012-2013, Guennadi Liakhovetski <lg@denx.de>
# (C) Copyright 2012-2013 Freescale Semiconductor, Inc.
# Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB    = $(obj)lib$(BOARD).o

COBJS  := nitrogen6x.o

SRCS   := $(COBJS:.o=.c)
OBJS   := $(addprefix $(obj),$(COBJS))

$(LIB):        $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
