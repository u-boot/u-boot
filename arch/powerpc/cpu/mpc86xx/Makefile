#
# Copyright 2007 Freescale Semiconductor, Inc.
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao,X.Xiao@motorola.com
#
# (C) Copyright 2004 Freescale Semiconductor. (MC86xx Port)
# Jeff Brown
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

SSTART	= start.o
CSTART	= traps.o

SOBJS-y += cache.o
SOBJS-$(CONFIG_MP) += release.o

COBJS-y	+= cpu.o
COBJS-y	+= cpu_init.o
# 8610 & 8641 are identical w/regards to DDR
COBJS-$(CONFIG_MPC8610) += ddr-8641.o
COBJS-$(CONFIG_MPC8641) += ddr-8641.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-y	+= interrupts.o
COBJS-$(CONFIG_MP) += mp.o
COBJS-$(CONFIG_MPC8610) += mpc8610_serdes.o
COBJS-$(CONFIG_MPC8641) += mpc8641_serdes.o
COBJS-y	+= speed.o

SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(SSTART) $(CSTART))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
