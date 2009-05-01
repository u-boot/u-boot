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

LIB	= $(obj)lib$(SOC).a

ifdef CONFIG_AT91CAP9
COBJS-$(CONFIG_MACB)		+= at91cap9_macb.o
COBJS-y				+= at91cap9_serial.o
COBJS-$(CONFIG_HAS_DATAFLASH)	+= at91cap9_spi.o
endif
ifdef CONFIG_AT91SAM9260
COBJS-$(CONFIG_MACB)		+= at91sam9260_macb.o
COBJS-y				+= at91sam9260_serial.o
COBJS-$(CONFIG_HAS_DATAFLASH)	+= at91sam9260_spi.o
endif
ifdef CONFIG_AT91SAM9G20
COBJS-$(CONFIG_MACB)		+= at91sam9260_macb.o
COBJS-y				+= at91sam9260_serial.o
COBJS-$(CONFIG_HAS_DATAFLASH)	+= at91sam9260_spi.o
endif
ifdef CONFIG_AT91SAM9261
COBJS-y				+= at91sam9261_serial.o
COBJS-$(CONFIG_HAS_DATAFLASH)	+= at91sam9261_spi.o
endif
ifdef CONFIG_AT91SAM9263
COBJS-$(CONFIG_MACB)		+= at91sam9263_macb.o
COBJS-y				+= at91sam9263_serial.o
COBJS-$(CONFIG_HAS_DATAFLASH)	+= at91sam9263_spi.o
COBJS-$(CONFIG_USB_OHCI_NEW)	+= at91sam9263_usb.o
endif
ifdef CONFIG_AT91SAM9RL
COBJS-y				+= at91sam9rl_serial.o
COBJS-$(CONFIG_HAS_DATAFLASH)	+= at91sam9rl_spi.o
endif
COBJS-$(CONFIG_AT91_LED)	+= led.o
COBJS-y += clock.o
COBJS-y += cpu.o
COBJS-y	+= timer.o

SRCS    := $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS    := $(addprefix $(obj),$(SOBJS) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
