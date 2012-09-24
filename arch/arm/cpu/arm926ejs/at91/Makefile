#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-$(CONFIG_AT91CAP9)	+= at91cap9_devices.o
COBJS-$(CONFIG_AT91SAM9260)	+= at91sam9260_devices.o
COBJS-$(CONFIG_AT91SAM9G20)	+= at91sam9260_devices.o
COBJS-$(CONFIG_AT91SAM9XE)	+= at91sam9260_devices.o
COBJS-$(CONFIG_AT91SAM9261)	+= at91sam9261_devices.o
COBJS-$(CONFIG_AT91SAM9G10)	+= at91sam9261_devices.o
COBJS-$(CONFIG_AT91SAM9263)	+= at91sam9263_devices.o
COBJS-$(CONFIG_AT91SAM9RL)	+= at91sam9rl_devices.o
COBJS-$(CONFIG_AT91SAM9M10G45)	+= at91sam9m10g45_devices.o
COBJS-$(CONFIG_AT91SAM9G45)	+= at91sam9m10g45_devices.o
COBJS-$(CONFIG_AT91SAM9X5)	+= at91sam9x5_devices.o
COBJS-$(CONFIG_AT91_EFLASH)	+= eflash.o
COBJS-$(CONFIG_AT91_LED)	+= led.o
COBJS-y += clock.o
COBJS-y += cpu.o
COBJS-y	+= reset.o
COBJS-y	+= timer.o

ifndef CONFIG_SKIP_LOWLEVEL_INIT
SOBJS-y	:= lowlevel_init.o
endif

SRCS    := $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS    := $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
