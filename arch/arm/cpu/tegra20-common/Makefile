#
# (C) Copyright 2010,2011 Nvidia Corporation.
#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

# The AVP is ARMv4T architecture so we must use special compiler
# flags for any startup files it might use.
CFLAGS_arch/arm/cpu/tegra20-common/warmboot_avp.o += -march=armv4t

LIB	= $(obj)lib$(SOC)-common.o

COBJS-y	+= clock.o funcmux.o pinmux.o
COBJS-$(CONFIG_TEGRA_LP0) += warmboot.o crypto.o warmboot_avp.o
COBJS-$(CONFIG_TEGRA_CLOCK_SCALING) += emc.o
COBJS-$(CONFIG_TEGRA_PMU) += pmu.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
