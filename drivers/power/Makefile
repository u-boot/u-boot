# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2009 Wind River Systems, Inc.
# Tom Rix <Tom.Rix at windriver.com>

obj-$(CONFIG_$(PHASE_)ACPI_PMC) += acpi_pmc/
obj-$(CONFIG_$(PHASE_)POWER_DOMAIN) += domain/
obj-y += pmic/
obj-y += regulator/

obj-$(CONFIG_AXP221_POWER)	+= axp221.o
ifdef CONFIG_XPL_BUILD
obj-$(CONFIG_AXP152_POWER)	+= axp152.o
obj-$(CONFIG_AXP209_POWER)	+= axp209.o
obj-$(CONFIG_AXP305_POWER)	+= axp_spl.o
obj-$(CONFIG_AXP313_POWER)	+= axp_spl.o
obj-$(CONFIG_AXP717_POWER)	+= axp_spl.o
obj-$(CONFIG_AXP809_POWER)	+= axp809.o
obj-$(CONFIG_AXP818_POWER)	+= axp818.o
obj-$(CONFIG_AXP803_POWER)	+= axp_spl.o
endif
obj-$(CONFIG_EXYNOS_TMU)	+= exynos-tmu.o
obj-$(CONFIG_SY8106A_POWER)	+= sy8106a.o
obj-$(CONFIG_TPS6586X_POWER)	+= tps6586x.o
obj-$(CONFIG_TWL4030_POWER)	+= twl4030.o
obj-$(CONFIG_PALMAS_POWER)	+= palmas.o
obj-$(CONFIG_$(PHASE_)POWER_LEGACY) += power_core.o
obj-$(CONFIG_DIALOG_POWER) += power_dialog.o
obj-$(CONFIG_POWER_FSL) += power_fsl.o
obj-$(CONFIG_$(PHASE_)POWER_I2C) += power_i2c.o
obj-$(CONFIG_POWER_SPI) += power_spi.o
obj-$(CONFIG_POWER_MT6323) += mt6323.o
