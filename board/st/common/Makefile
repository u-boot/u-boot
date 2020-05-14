# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (C) 2020, STMicroelectronics - All Rights Reserved
#

obj-$(CONFIG_CMD_STBOARD) += cmd_stboard.o

ifeq ($(CONFIG_ARCH_STM32MP),y)
obj-$(CONFIG_SYS_MTDPARTS_RUNTIME) += stm32mp_mtdparts.o
obj-$(CONFIG_SET_DFU_ALT_INFO) += stm32mp_dfu.o
endif
