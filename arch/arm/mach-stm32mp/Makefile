# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

obj-y += dram_init.o
obj-y += syscon.o
obj-y += bsec.o
obj-y += soc.o

obj-$(CONFIG_STM32MP15X) += stm32mp1/
obj-$(CONFIG_STM32MP13X) += stm32mp1/
obj-$(CONFIG_STM32MP23X) += stm32mp2/
obj-$(CONFIG_STM32MP25X) += stm32mp2/

obj-$(CONFIG_MFD_STM32_TIMERS) += timers.o
obj-$(CONFIG_STM32MP_TAMP_NVMEM) += tamp_nvram.o

obj-$(CONFIG_STM32_ECDSA_VERIFY) += ecdsa_romapi.o
ifndef CONFIG_XPL_BUILD
obj-y += cmd_stm32prog/
obj-$(CONFIG_CMD_STM32KEY) += cmd_stm32key.o
obj-$(CONFIG_TFABOOT) += boot_params.o
endif
