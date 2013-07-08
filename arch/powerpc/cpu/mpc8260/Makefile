#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

START	= start.o kgdb.o
COBJS	= traps.o serial_smc.o serial_scc.o cpu.o cpu_init.o speed.o \
	  interrupts.o ether_fcc.o i2c.o commproc.o \
	  bedbug_603e.o pci.o spi.o

COBJS-$(CONFIG_ETHER_ON_SCC) = ether_scc.o

COBJS	+= $(COBJS-y)

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS) $(obj)kgdb.o)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
