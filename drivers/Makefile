#
# (C) Copyright 2000
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
	  bcm570x.o bcm570x_autoneg.o cfb_console.o \
	  cs8900.o ct69000.o dataflash.o dc2114x.o          \
	  e1000.o eepro100.o \
	  i8042.o i82365.o inca-ip_sw.o \
	  lan91c96.o             natsemi.o \
	  ns16550.o ns8382x.o ns87308.o \
	  pci.o pci_auto.o pci_indirect.o \
	  pcnet.o plb2800_eth.o \
	  s3c24x0_i2c.o sed13806.o serial.o \
	  serial_max3100.o \
	  smc91111.o smiLynxEM.o              sym53c8xx.o \
	  ti_pci1410a.o tigon3.o w83c553f.o \
	  status_led.o

## Disabled for now:
##	  cs8900.o ct69000.o dataflash.o dc2114x.o ds1722.o \
##	  lan91c96.o mw_eeprom.o natsemi.o \
##	  smc91111.o smiLynxEM.o spi_eeprom.o sym53c8xx.o \
##

all:	$(LIB)

$(LIB):	$(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
