#
# (C) Copyright 2007-2009 DENX Software Engineering
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

$(shell mkdir -p $(OBJTREE)/board/freescale/common)

LIB	= $(obj)lib$(CPU).a

START	= start.o
COBJS-y	:= cpu.o
COBJS-y	+= traps.o
COBJS-y += common.o
COBJS-y += cpu_init.o
COBJS-y += fixed_sdram.o
COBJS-y += i2c.o
COBJS-y += interrupts.o
COBJS-y += iopin.o
COBJS-y += serial.o
COBJS-y += speed.o
COBJS-$(CONFIG_FSL_DIU_FB) += diu.o
COBJS-$(CONFIG_FSL_DIU_FB) += ../../../../board/freescale/common/fsl_diu_fb.o
COBJS-$(CONFIG_FSL_DIU_FB) += ../../../../board/freescale/common/fsl_logo_bmp.o
COBJS-$(CONFIG_CMD_IDE) += ide.o
COBJS-$(CONFIG_IIM) += iim.o
COBJS-$(CONFIG_PCI) += pci.o

COBJS	:= $(COBJS-y)
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
