#
# (C) Copyright 2000-2004
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

LIB	= libdrivers.a

OBJS	= 3c589.o 5701rls.o ali512x.o \
	  bcm570x.o bcm570x_autoneg.o cfb_console.o cfi_flash.o \
	  cs8900.o ct69000.o dataflash.o dc2114x.o dm9000x.o \
	  e1000.o eepro100.o \
	  i8042.o i82365.o inca-ip_sw.o keyboard.o \
	  lan91c96.o \
	  natsemi.o ne2000.o netarm_eth.o netconsole.o \
	  ns16550.o ns8382x.o ns87308.o ns7520_eth.o omap1510_i2c.o \
	  omap24xx_i2c.o pci.o pci_auto.o pci_indirect.o \
	  pcnet.o plb2800_eth.o \
	  ps2ser.o ps2mult.o pc_keyb.o \
	  rtl8019.o rtl8139.o rtl8169.o \
	  s3c4510b_eth.o s3c4510b_uart.o \
	  sed13806.o sed156x.o \
	  serial.o serial_max3100.o \
	  serial_pl010.o serial_pl011.o serial_xuartlite.o \
	  sl811_usb.o sm501.o smc91111.o smiLynxEM.o \
	  status_led.o sym53c8xx.o \
	  ti_pci1410a.o tigon3.o tsec.o \
	  usbdcore.o usbdcore_ep0.o usbdcore_omap1510.o usbtty.o \
	  videomodes.o w83c553f.o \
	  ks8695eth.o

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
