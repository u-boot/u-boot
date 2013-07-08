#
# (C) Copyright 2007-2009 DENX Software Engineering
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

$(shell mkdir -p $(OBJTREE)/board/freescale/common)

LIB	= $(obj)lib$(CPU).o

START	= start.o
COBJS-y	:= cpu.o
COBJS-y	+= traps.o
COBJS-y += cpu_init.o
COBJS-y += fixed_sdram.o
COBJS-y += i2c.o
COBJS-y += interrupts.o
COBJS-y += iopin.o
COBJS-y += serial.o
COBJS-y += speed.o
COBJS-$(CONFIG_FSL_DIU_FB) += diu.o
COBJS-$(CONFIG_CMD_IDE) += ide.o
COBJS-$(CONFIG_PCI) += pci.o

# Stub implementations of cache management functions for USB
COBJS-$(CONFIG_USB_EHCI) += cache.o

COBJS	:= $(COBJS-y)
SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
