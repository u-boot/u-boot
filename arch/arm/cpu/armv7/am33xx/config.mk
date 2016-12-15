#
# Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#

include  $(srctree)/$(CPUDIR)/omap-common/config_secure.mk

ifdef CONFIG_SPL_BUILD
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
#
# For booting from SPI use
# u-boot-spl_HS_SPI_X-LOADER to program flash
#
# For booting spl from all other  media
# use u-boot-spl_HS_ISSW
#
# Refer to README.ti-secure for more info
#
ALL-y	+= u-boot-spl_HS_ISSW
ALL-$(CONFIG_SPL_SPI_SUPPORT) += u-boot-spl_HS_SPI_X-LOADER
else
ALL-y	+= MLO
ALL-$(CONFIG_SPL_SPI_SUPPORT) += MLO.byteswap
endif
else
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
ALL-$(CONFIG_QSPI_BOOT) += u-boot_HS_XIP_X-LOADER
ALL-$(CONFIG_SPL_LOAD_FIT) += u-boot_HS.img
endif
ALL-y	+= u-boot.img
endif
