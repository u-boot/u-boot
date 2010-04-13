#
# Copyright 2007 Freescale Semiconductor, Inc.
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao,X.Xiao@motorola.com
#
# (C) Copyright 2004 Freescale Semiconductor. (MC86xx Port)
# Jeff Brown
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).a

START	= start.o

SOBJS-y += cache.o
SOBJS-$(CONFIG_MP) += release.o

COBJS-y	+= cpu.o
COBJS-y	+= cpu_init.o
# 8610 & 8641 are identical w/regards to DDR
COBJS-$(CONFIG_MPC8610) += ddr-8641.o
COBJS-$(CONFIG_MPC8641) += ddr-8641.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-y	+= interrupts.o
COBJS-$(CONFIG_MP) += mp.o
COBJS-y	+= speed.o
COBJS-y	+= traps.o

SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(ASOBJS) $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
