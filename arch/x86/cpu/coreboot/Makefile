#
# Copyright (c) 2011 The Chromium OS Authors.
#
# (C) Copyright 2008
# Graeme Russ, graeme.russ@gmail.com.
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002
# Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)lib$(SOC).o

SOBJS-$(CONFIG_SYS_COREBOOT) += car.o
COBJS-$(CONFIG_SYS_COREBOOT) += coreboot.o
COBJS-$(CONFIG_SYS_COREBOOT) += tables.o
COBJS-$(CONFIG_SYS_COREBOOT) += ipchecksum.o
COBJS-$(CONFIG_SYS_COREBOOT) += sdram.o
COBJS-$(CONFIG_SYS_COREBOOT) += timestamp.o
COBJS-$(CONFIG_PCI) += pci.o

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all: $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
