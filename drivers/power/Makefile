#
# Copyright (c) 2009 Wind River Systems, Inc.
# Tom Rix <Tom.Rix at windriver.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libpower.o

COBJS-$(CONFIG_EXYNOS_TMU)	+= exynos-tmu.o
COBJS-$(CONFIG_FTPMU010_POWER)	+= ftpmu010.o
COBJS-$(CONFIG_TPS6586X_POWER)	+= tps6586x.o
COBJS-$(CONFIG_TWL4030_POWER)	+= twl4030.o
COBJS-$(CONFIG_TWL6030_POWER)	+= twl6030.o
COBJS-$(CONFIG_PALMAS_POWER)	+= palmas.o

COBJS-$(CONFIG_POWER) += power_core.o
COBJS-$(CONFIG_DIALOG_POWER) += power_dialog.o
COBJS-$(CONFIG_POWER_FSL) += power_fsl.o
COBJS-$(CONFIG_POWER_I2C) += power_i2c.o
COBJS-$(CONFIG_POWER_SPI) += power_spi.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

########################################################################
