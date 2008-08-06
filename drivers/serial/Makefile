#
# (C) Copyright 2006
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

LIB	:= $(obj)libserial.a

COBJS-$(CONFIG_ATMEL_USART) += atmel_usart.o
COBJS-$(CONFIG_MCFUART) += mcfuart.o
COBJS-y += ns9750_serial.o
COBJS-y += ns16550.o
COBJS-$(CONFIG_DRIVER_S3C4510_UART) += s3c4510b_uart.o
COBJS-y += serial.o
COBJS-$(CONFIG_MAX3100_SERIAL) += serial_max3100.o
COBJS-y += serial_pl010.o
COBJS-y += serial_pl011.o
COBJS-$(CONFIG_XILINX_UARTLITE) += serial_xuartlite.o
COBJS-y += serial_sh.o
COBJS-$(CONFIG_USB_TTY) += usbtty.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
