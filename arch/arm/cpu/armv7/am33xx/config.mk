#
# Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#
ifdef CONFIG_SPL_BUILD
ALL-y	+= $(OBJTREE)/MLO
ALL-$(CONFIG_SPL_SPI_SUPPORT) += $(OBJTREE)/MLO.byteswap
else
ALL-y	+= $(obj)u-boot.img
endif
