#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2003
# Martin Winistoerfer, martinwinistoerfer@gmx.ch.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# File:			arch/powerpc/cpu/mpc5xx/Makefile
#
# Discription:		Makefile to build mpc5xx cpu configuration.
#			Will include top config.mk which itselfs
#			uses the definitions made in arch/powerpc/cpu/mpc5xx/config.mk
#


include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

START	= start.o
COBJS	= serial.o cpu.o cpu_init.o interrupts.o traps.o speed.o spi.o

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
