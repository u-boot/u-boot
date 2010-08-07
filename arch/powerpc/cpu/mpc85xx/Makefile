#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao,X.Xiao@motorola.com
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

START	= start.o resetvec.o
SOBJS-$(CONFIG_MP)	+= release.o
SOBJS	= $(SOBJS-y)

COBJS-$(CONFIG_CMD_ERRATA) += cmd_errata.o
COBJS-$(CONFIG_CPM2)	+= commproc.o

# supports ddr1
COBJS-$(CONFIG_MPC8540) += ddr-gen1.o
COBJS-$(CONFIG_MPC8560) += ddr-gen1.o
COBJS-$(CONFIG_MPC8541) += ddr-gen1.o
COBJS-$(CONFIG_MPC8555) += ddr-gen1.o

# supports ddr1/2
COBJS-$(CONFIG_MPC8548) += ddr-gen2.o
COBJS-$(CONFIG_MPC8568) += ddr-gen2.o
COBJS-$(CONFIG_MPC8544) += ddr-gen2.o

# supports ddr1/2/3
COBJS-$(CONFIG_MPC8572) += ddr-gen3.o
COBJS-$(CONFIG_MPC8536) += ddr-gen3.o
COBJS-$(CONFIG_MPC8569)	+= ddr-gen3.o
COBJS-$(CONFIG_P1011)	+= ddr-gen3.o
COBJS-$(CONFIG_P1012)	+= ddr-gen3.o
COBJS-$(CONFIG_P1013)	+= ddr-gen3.o
COBJS-$(CONFIG_P1020)	+= ddr-gen3.o
COBJS-$(CONFIG_P1021)	+= ddr-gen3.o
COBJS-$(CONFIG_P1022)	+= ddr-gen3.o
COBJS-$(CONFIG_P2010)	+= ddr-gen3.o
COBJS-$(CONFIG_P2020)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P3041)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P4080)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P5020)	+= ddr-gen3.o

COBJS-$(CONFIG_CPM2)	+= ether_fcc.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-$(CONFIG_FSL_CORENET) += liodn.o
COBJS-$(CONFIG_MP)	+= mp.o
COBJS-$(CONFIG_MPC8536) += mpc8536_serdes.o
COBJS-$(CONFIG_P1022)	+= p1022_serdes.o
COBJS-$(CONFIG_PCI)	+= pci.o
COBJS-$(CONFIG_FSL_CORENET) += portals.o

# various SoC specific assignments
COBJS-$(CONFIG_PPC_P4080) += p4080_ids.o

COBJS-$(CONFIG_QE)	+= qe_io.o
COBJS-$(CONFIG_CPM2)	+= serial_scc.o
COBJS-$(CONFIG_FSL_CORENET) += fsl_corenet_serdes.o
COBJS-$(CONFIG_PPC_P4080) += p4080_serdes.o

COBJS	= $(COBJS-y)
COBJS	+= cpu.o
COBJS	+= cpu_init.o
COBJS	+= cpu_init_early.o
COBJS	+= interrupts.o
COBJS	+= speed.o
COBJS	+= tlb.o
COBJS	+= traps.o

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
