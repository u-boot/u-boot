#
# (C) Copyright 2000-2007
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG

LIB	= $(obj)libdrivers.a

COBJS-y += ali512x.o
COBJS-y += ds1722.o
COBJS-y += ns87308.o
COBJS-y += status_led.o

#
# Block and Flash Drivers
#
COBJS-y += ahci.o
COBJS-y += at45.o
COBJS-y += ata_piix.o
COBJS-y += cfi_flash.o
COBJS-y += dataflash.o
COBJS-y += mw_eeprom.o
COBJS-y += sil680.o
COBJS-y += sym53c8xx.o
COBJS-y += systemace.o

#
# Console Drivers
#
COBJS-y += i8042.o
COBJS-y += keyboard.o
COBJS-y += pc_keyb.o
COBJS-y += ps2ser.o
COBJS-y += ps2mult.o

#
# USB Drivers
#
COBJS-y += isp116x-hcd.o
COBJS-y += sl811_usb.o
COBJS-y += usb_ohci.o
COBJS-y += usbdcore.o
COBJS-y += usbdcore_ep0.o
COBJS-y += usbdcore_mpc8xx.o
COBJS-y += usbdcore_omap1510.o

#
# Miscellaneous Drivers
#
COBJS-y += ali512x.o
COBJS-y += ns87308.o
COBJS-y += status_led.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
