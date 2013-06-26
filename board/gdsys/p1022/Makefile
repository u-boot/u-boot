#
# Copyright 2010 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

# COBJS-y	+= $(BOARD).o
COBJS-y	+= law.o
COBJS-y	+= ddr.o
COBJS-y	+= tlb.o
COBJS-y	+= sdhc_boot.o
COBJS-$(CONFIG_CONTROLCENTERD) += controlcenterd.o controlcenterd-id.o

COBJS-$(CONFIG_FSL_DIU_FB) += diu.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
