# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

obj-y += cpu.o
obj-y += dram_init.o
obj-y += syscon.o
obj-y += bsec.o

obj-$(CONFIG_STM32MP13x) += stm32mp13x.o
obj-$(CONFIG_STM32MP15x) += stm32mp15x.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += tzc400.o
obj-$(CONFIG_STM32_ECDSA_VERIFY) += ecdsa_romapi.o
else
obj-y += cmd_stm32prog/
obj-$(CONFIG_CMD_STM32KEY) += cmd_stm32key.o
obj-$(CONFIG_ARMV7_PSCI) += psci.o
obj-$(CONFIG_TFABOOT) += boot_params.o
endif

obj-$(CONFIG_$(SPL_)STM32MP15_PWR) += pwr_regulator.o
obj-$(CONFIG_OF_SYSTEM_SETUP) += fdt.o
