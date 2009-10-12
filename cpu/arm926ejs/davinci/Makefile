#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
#
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

LIB	= $(obj)lib$(SOC).a

COBJS-y				+= cpu.o timer.o psc.o
COBJS-$(CONFIG_SOC_DM355)	+= dm355.o
COBJS-$(CONFIG_SOC_DM365)	+= dm365.o
COBJS-$(CONFIG_SOC_DM644X)	+= dm644x.o
COBJS-$(CONFIG_SOC_DM646X)	+= dm646x.o
COBJS-$(CONFIG_DRIVER_TI_EMAC)	+= lxt972.o dp83848.o

SOBJS	= reset.o

ifndef CONFIG_SKIP_LOWLEVEL_INIT
SOBJS	+= lowlevel_init.o
endif

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y) $(SOBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
