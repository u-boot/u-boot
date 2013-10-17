#
# Cirrus Logic EP93xx CPU-specific Makefile
#
# Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
#
# Copyright (C) 2004, 2005
# Cory T. Tusar, Videon Central, Inc., <ctusar@videon-central.com>
#
# Copyright (C) 2006
# Dominic Rath <Dominic.Rath@gmx.de>
#
# Based on an original Makefile, which is
#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
include $(TOPDIR)/config.mk

LIB = $(obj)lib$(SOC).o

COBJS   = cpu.o led.o speed.o timer.o
SOBJS   = lowlevel_init.o

SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS    := $(addprefix $(obj),$(SOBJS) $(COBJS))

all:    $(obj).depend $(LIB)

$(LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
