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
COBJS-y += ati_radeon_fb.o
COBJS-y += atmel_usart.o
COBJS-y += cfb_console.o
COBJS-y += ct69000.o
COBJS-y += i8042.o
COBJS-y += keyboard.o
COBJS-y += netconsole.o
COBJS-y += ns16550.o
COBJS-y += pc_keyb.o
COBJS-y += ps2ser.o
COBJS-y += ps2mult.o
COBJS-y += s3c4510b_uart.o
COBJS-y += sed13806.o
COBJS-y += sed156x.o
COBJS-y += serial.o
COBJS-y += serial_max3100.o
COBJS-y += serial_xuartlite.o
COBJS-y += serial_sh.o
COBJS-y += sm501.o
COBJS-y += smiLynxEM.o
COBJS-y += usbtty.o
COBJS-y += videomodes.o

#
# I2C Drivers
#
COBJS-y += omap1510_i2c.o
COBJS-y += omap24xx_i2c.o
COBJS-y += tsi108_i2c.o
COBJS-y += fsl_i2c.o

#
# Network Drivers
#
COBJS-y += 3c589.o
COBJS-y += bcm570x.o bcm570x_autoneg.o 5701rls.o
COBJS-y += cs8900.o
COBJS-y += dc2114x.o
COBJS-y += dm9000x.o
COBJS-y += e1000.o
COBJS-y += eepro100.o
COBJS-y += enc28j60.o
COBJS-y += inca-ip_sw.o
COBJS-y += ks8695eth.o
COBJS-y += lan91c96.o
COBJS-y += macb.o
COBJS-y += natsemi.o
COBJS-y += ne2000.o
COBJS-y += netarm_eth.o
COBJS-y += ns7520_eth.o
COBJS-y += ns8382x.o
COBJS-y += pcnet.o
COBJS-y += plb2800_eth.o
COBJS-y += rtl8019.o
COBJS-y += rtl8139.o
COBJS-y += rtl8169.o
COBJS-y += s3c4510b_eth.o
COBJS-y += smc91111.o
COBJS-y += tigon3.o
COBJS-y += tsec.o
COBJS-y += tsi108_eth.o
COBJS-y += uli526x.o

#
# PCI/PCMCIA device drivers
#
COBJS-y += fsl_pci_init.o
COBJS-y += mpc8xx_pcmcia.o
COBJS-y += pci.o
COBJS-y += pci_auto.o
COBJS-y += pci_indirect.o
COBJS-y += pxa_pcmcia.o
COBJS-y += rpx_pcmcia.o
COBJS-y += ti_pci1410a.o
COBJS-y += tqm8xx_pcmcia.o
COBJS-y += tsi108_pci.o
COBJS-y += w83c553f.o

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
