#
# Copyright 2009-2010 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib8xxx.o

ifneq ($(CPU),mpc83xx)
COBJS-y	+= cpu.o
endif

COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-$(CONFIG_FSL_IFC) += fsl_ifc.o
COBJS-$(CONFIG_FSL_LBC) += fsl_lbc.o
COBJS-$(CONFIG_SYS_SRIO) += srio.o

SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

include $(SRCTREE)/rules.mk

sinclude $(obj).depend
