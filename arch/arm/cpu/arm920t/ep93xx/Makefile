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
# See file CREDITS for list of people who contributed to this project.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
include $(TOPDIR)/config.mk

LIB = $(obj)lib$(SOC).a

COBJS   = cpu.o led.o speed.o timer.o
SOBJS   = lowlevel_init.o

SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS    := $(addprefix $(obj),$(SOBJS) $(COBJS))

all:    $(obj).depend $(LIB)

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
