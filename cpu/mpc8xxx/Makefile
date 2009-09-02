#
# Copyright 2009 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib8xxx.a

COBJS-y	+= cpu.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-$(CONFIG_PCI)	+= pci_cfg.o

SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

include $(SRCTREE)/rules.mk

sinclude $(obj).depend
