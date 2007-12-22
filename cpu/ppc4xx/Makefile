#
# (C) Copyright 2000-2006
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

LIB	= $(obj)lib$(CPU).a

START	:= resetvec.o
START	+= start.o

SOBJS	:= cache.o
SOBJS	+= dcr.o
SOBJS	+= kgdb.o

COBJS	:= 40x_spd_sdram.o
COBJS	+= 44x_spd_ddr.o
COBJS	+= 44x_spd_ddr2.o
COBJS	+= 4xx_enet.o
COBJS	+= 4xx_pci.o
COBJS	+= 4xx_pcie.o
COBJS	+= 4xx_uart.o
COBJS	+= bedbug_405.o
COBJS	+= commproc.o
COBJS	+= cpu.o
COBJS	+= cpu_init.o
COBJS	+= denali_data_eye.o
COBJS	+= denali_spd_ddr2.o
COBJS	+= fdt.o
COBJS	+= gpio.o
COBJS	+= i2c.o
COBJS	+= interrupts.o
COBJS	+= iop480_uart.o
COBJS	+= miiphy.o
COBJS	+= ndfc.o
COBJS	+= sdram.o
COBJS	+= speed.o
COBJS	+= tlb.o
COBJS	+= traps.o
COBJS	+= usb.o
COBJS	+= usb_ohci.o
COBJS	+= usbdev.o

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
