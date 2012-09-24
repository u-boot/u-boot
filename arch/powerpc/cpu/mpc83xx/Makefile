#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright 2004 Freescale Semiconductor, Inc.
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

LIB	= $(obj)lib$(CPU).o

START	= start.o

COBJS-y += traps.o
COBJS-y += cpu.o
COBJS-y += cpu_init.o
COBJS-y += speed.o
COBJS-y += interrupts.o
COBJS-y += ecc.o
COBJS-$(CONFIG_QE) += qe_io.o
COBJS-$(CONFIG_FSL_SERDES) += serdes.o
COBJS-$(CONFIG_PCI) += pci.o
COBJS-$(CONFIG_PCIE) += pcie.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o

# Stub implementations of cache management functions for USB
COBJS-y += cache.o

ifdef CONFIG_FSL_DDR2
COBJS_LN-$(CONFIG_MPC8349) += ddr-gen2.o
else
COBJS-y += spd_sdram.o
endif
COBJS-$(CONFIG_FSL_DDR2) += law.o

COBJS	:= $(COBJS-y)
SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c) $(addprefix $(obj),$(COBJS_LN-y:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS) $(COBJS_LN-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(obj)ddr-gen1.c:
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/ddr-gen1.c $(obj)ddr-gen1.c

$(obj)ddr-gen2.c:
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/ddr-gen2.c $(obj)ddr-gen2.c

$(obj)ddr-gen3.c:
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/ddr-gen3.c $(obj)ddr-gen3.c

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
