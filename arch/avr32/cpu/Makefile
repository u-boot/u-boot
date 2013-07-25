#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2005-2006 Atmel Corporation.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)lib$(CPU).o

START-y			+= start.o

COBJS-y			+= cpu.o
COBJS-$(CONFIG_SYS_HSDRAMC) += hsdramc.o
COBJS-y			+= exception.o
COBJS-y			+= cache.o
COBJS-y			+= interrupts.o
COBJS-$(CONFIG_PORTMUX_PIO) += portmux-pio.o
COBJS-$(CONFIG_PORTMUX_GPIO) += portmux-gpio.o

SRCS	:= $(START-y:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(START-y))

all: $(obj).depend $(START) $(LIB)

$(LIB): $(OBJS)
	$(call cmd_link_o_target, $^)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
