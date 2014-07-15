#
# Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#
ifdef CONFIG_SPL_BUILD
ALL-y	+= MLO
ALL-$(CONFIG_SPL_SPI_SUPPORT) += MLO.byteswap
else
ALL-y	+= u-boot.img
endif
