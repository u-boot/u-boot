# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

obj-y += cpu.o
obj-y += etzpc.o

obj-$(CONFIG_STM32MP13X) += stm32mp13x.o
obj-$(CONFIG_STM32MP15X) += stm32mp15x.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o
obj-y += tzc400.o
else
obj-$(CONFIG_ARMV7_PSCI) += psci.o
endif

obj-$(CONFIG_$(PHASE_)STM32MP15_PWR) += pwr_regulator.o
obj-$(CONFIG_OF_SYSTEM_SETUP) += fdt.o
