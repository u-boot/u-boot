#
# Copyright 2008 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libddr.a

COBJS-$(CONFIG_FSL_DDR1)	+= main.o util.o ctrl_regs.o options.o \
				   lc_common_dimm_params.o
COBJS-$(CONFIG_FSL_DDR1)	+= ddr1_dimm_params.o

COBJS-$(CONFIG_FSL_DDR2)	+= main.o util.o ctrl_regs.o options.o \
				   lc_common_dimm_params.o
COBJS-$(CONFIG_FSL_DDR2)	+= ddr2_dimm_params.o

COBJS-$(CONFIG_FSL_DDR3)	+= main.o util.o ctrl_regs.o options.o \
				   lc_common_dimm_params.o
COBJS-$(CONFIG_FSL_DDR3)	+= ddr3_dimm_params.o

SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

include $(SRCTREE)/rules.mk

sinclude $(obj).depend
