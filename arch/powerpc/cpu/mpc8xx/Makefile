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

# CFLAGS += -DET_DEBUG

LIB	= $(obj)lib$(CPU).o

SSTART-y += start.o
CSTART-y += traps.o
COBJS-y	+= bedbug_860.o
COBJS-y	+= commproc.o
COBJS-y	+= cpu.o
COBJS-y	+= cpu_init.o
COBJS-y	+= fec.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-y	+= i2c.o
COBJS-y	+= interrupts.o
COBJS-y	+= lcd.o
COBJS-y	+= scc.o
COBJS-y	+= serial.o
COBJS-y	+= speed.o
COBJS-y	+= spi.o
COBJS-y	+= upatch.o
COBJS-y	+= video.o
SOBJS-y	+= kgdb.o
SOBJS-y	+= plprcr_write.o

SRCS	:= $(SSTART-y:.o=.S) $(CSTART-y:.o=.c) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(SSTART-y) $(CSTART-y))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
