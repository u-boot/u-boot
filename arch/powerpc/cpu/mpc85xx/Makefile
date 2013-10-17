#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao,X.Xiao@motorola.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

START = start.o resetvec.o

ifdef MINIMAL

COBJS-y	+= cpu_init_early.o tlb.o spl_minimal.o

else

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
COBJS-$(CONFIG_PPC_C29X)	+= ddr-gen3.o
COBJS-$(CONFIG_MPC8572) += ddr-gen3.o
COBJS-$(CONFIG_MPC8536) += ddr-gen3.o
COBJS-$(CONFIG_MPC8569)	+= ddr-gen3.o
COBJS-$(CONFIG_P1010)	+= ddr-gen3.o
COBJS-$(CONFIG_P1011)	+= ddr-gen3.o
COBJS-$(CONFIG_P1012)	+= ddr-gen3.o
COBJS-$(CONFIG_P1013)	+= ddr-gen3.o
COBJS-$(CONFIG_P1014)	+= ddr-gen3.o
COBJS-$(CONFIG_P1020)	+= ddr-gen3.o
COBJS-$(CONFIG_P1021)	+= ddr-gen3.o
COBJS-$(CONFIG_P1022)	+= ddr-gen3.o
COBJS-$(CONFIG_P1023)	+= ddr-gen3.o
COBJS-$(CONFIG_P1024)	+= ddr-gen3.o
COBJS-$(CONFIG_P1025)	+= ddr-gen3.o
COBJS-$(CONFIG_P2010)	+= ddr-gen3.o
COBJS-$(CONFIG_P2020)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P2041)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P3041)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P4080)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P5020)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_P5040)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_T4240)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_T4160)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_B4420)	+= ddr-gen3.o
COBJS-$(CONFIG_PPC_B4860)	+= ddr-gen3.o
COBJS-$(CONFIG_BSC9131)		+= ddr-gen3.o
COBJS-$(CONFIG_BSC9132)		+= ddr-gen3.o
COBJS-$(CONFIG_PPC_T1040)	+= ddr-gen3.o

COBJS-$(CONFIG_CPM2)	+= ether_fcc.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-$(CONFIG_FSL_CORENET) += liodn.o
COBJS-$(CONFIG_MP)	+= mp.o
COBJS-$(CONFIG_PCI)	+= pci.o
COBJS-$(CONFIG_SYS_DPAA_QBMAN) += portals.o

# various SoC specific assignments
COBJS-$(CONFIG_PPC_P2041) += p2041_ids.o
COBJS-$(CONFIG_PPC_P3041) += p3041_ids.o
COBJS-$(CONFIG_PPC_P4080) += p4080_ids.o
COBJS-$(CONFIG_PPC_P5020) += p5020_ids.o
COBJS-$(CONFIG_PPC_P5040) += p5040_ids.o
COBJS-$(CONFIG_PPC_T4240) += t4240_ids.o
COBJS-$(CONFIG_PPC_T4160) += t4240_ids.o
COBJS-$(CONFIG_PPC_B4420) += b4860_ids.o
COBJS-$(CONFIG_PPC_B4860) += b4860_ids.o
COBJS-$(CONFIG_PPC_T1040) += t1040_ids.o

COBJS-$(CONFIG_QE)	+= qe_io.o
COBJS-$(CONFIG_CPM2)	+= serial_scc.o
COBJS-$(CONFIG_SYS_FSL_QORIQ_CHASSIS1) += fsl_corenet_serdes.o
COBJS-$(CONFIG_SYS_FSL_QORIQ_CHASSIS2) += fsl_corenet2_serdes.o

# SoC specific SERDES support
COBJS-$(CONFIG_PPC_C29X)	+= c29x_serdes.o
COBJS-$(CONFIG_MPC8536) += mpc8536_serdes.o
COBJS-$(CONFIG_MPC8544) += mpc8544_serdes.o
COBJS-$(CONFIG_MPC8548) += mpc8548_serdes.o
COBJS-$(CONFIG_MPC8568) += mpc8568_serdes.o
COBJS-$(CONFIG_MPC8569) += mpc8569_serdes.o
COBJS-$(CONFIG_MPC8572) += mpc8572_serdes.o
COBJS-$(CONFIG_P1010)	+= p1010_serdes.o
COBJS-$(CONFIG_P1011)	+= p1021_serdes.o
COBJS-$(CONFIG_P1012)	+= p1021_serdes.o
COBJS-$(CONFIG_P1013)	+= p1022_serdes.o
COBJS-$(CONFIG_P1014)	+= p1010_serdes.o
COBJS-$(CONFIG_P1017)	+= p1023_serdes.o
COBJS-$(CONFIG_P1020)	+= p1021_serdes.o
COBJS-$(CONFIG_P1021)	+= p1021_serdes.o
COBJS-$(CONFIG_P1022)	+= p1022_serdes.o
COBJS-$(CONFIG_P1023)	+= p1023_serdes.o
COBJS-$(CONFIG_P1024)	+= p1021_serdes.o
COBJS-$(CONFIG_P1025)	+= p1021_serdes.o
COBJS-$(CONFIG_P2010)	+= p2020_serdes.o
COBJS-$(CONFIG_P2020)	+= p2020_serdes.o
COBJS-$(CONFIG_PPC_P2041) += p2041_serdes.o
COBJS-$(CONFIG_PPC_P3041) += p3041_serdes.o
COBJS-$(CONFIG_PPC_P4080) += p4080_serdes.o
COBJS-$(CONFIG_PPC_P5020) += p5020_serdes.o
COBJS-$(CONFIG_PPC_P5040) += p5040_serdes.o
COBJS-$(CONFIG_PPC_T4240) += t4240_serdes.o
COBJS-$(CONFIG_PPC_T4160) += t4240_serdes.o
COBJS-$(CONFIG_PPC_B4420) += b4860_serdes.o
COBJS-$(CONFIG_PPC_B4860) += b4860_serdes.o
COBJS-$(CONFIG_BSC9132) += bsc9132_serdes.o
COBJS-$(CONFIG_PPC_T1040) += t1040_serdes.o

COBJS-y	+= cpu.o
COBJS-y	+= cpu_init.o
COBJS-y	+= cpu_init_early.o
COBJS-y	+= interrupts.o
COBJS-y	+= speed.o
COBJS-y	+= tlb.o
COBJS-y	+= traps.o

# Stub implementations of cache management functions for USB
COBJS-y += cache.o

endif # not minimal

COBJS	= $(COBJS-y)

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
