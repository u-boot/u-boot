#
# Copyright 2007-2009 Freescale Semiconductor, Inc.
# (C) Copyright 2001-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-y	+= $(BOARD).o
COBJS-y	+= ddr.o
COBJS-$(CONFIG_P3041DS)	+= eth_hydra.o
COBJS-$(CONFIG_P4080DS)	+= eth_p4080.o
COBJS-$(CONFIG_P5020DS)	+= eth_hydra.o
COBJS-$(CONFIG_P5040DS)	+= eth_superhydra.o
COBJS-$(CONFIG_P3041DS)	+= p3041ds_ddr.o
COBJS-$(CONFIG_P4080DS)	+= p4080ds_ddr.o
COBJS-$(CONFIG_P5020DS)	+= p5020ds_ddr.o
COBJS-$(CONFIG_P5040DS)	+= p5040ds_ddr.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
