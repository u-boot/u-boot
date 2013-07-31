#
# Copyright (C) 2009 Samsung Electronics
# Minkyu Kang <mk7.kang@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-y	+= clock.o power.o soc.o system.o pinmux.o tzpc.o

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_EXYNOS5)	+= clock_init_exynos5.o
COBJS-$(CONFIG_EXYNOS5)	+= dmc_common.o dmc_init_ddr3.o
COBJS-$(CONFIG_EXYNOS4210)+= dmc_init_exynos4.o clock_init_exynos4.o
COBJS-y	+= spl_boot.o
COBJS-y	+= lowlevel_init.o
endif

COBJS   := $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
