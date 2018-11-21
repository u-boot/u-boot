# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

ALL-$(CONFIG_SPL_BUILD) += spl/u-boot-spl.stm32

MKIMAGEFLAGS_u-boot-spl.stm32 = -T stm32image -a $(CONFIG_SPL_TEXT_BASE) -e $(CONFIG_SPL_TEXT_BASE)

spl/u-boot-spl.stm32: MKIMAGEOUTPUT = spl/u-boot-spl.stm32.log

spl/u-boot-spl.stm32: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)
