#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright 2004 Freescale Semiconductor, Inc.
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

START	= start.o

ifdef MINIMAL

COBJS-y	+= spl_minimal.o

else

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

endif # not minimal

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
