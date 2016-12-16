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
# On AM43XX:
#
# For booting spl from all other media use
# u-boot-spl_HS_ISSW
#
# On AM33XX:
#
# For booting spl from NAND flash use
# u-boot-spl_HS_X-LOADER
#
# For booting spl from SD/MMC/eMMC media use
# u-boot-spl_HS_MLO
#
# For booting spl over UART, USB, or Ethernet use
# u-boot-spl_HS_2ND
#
# Refer to README.ti-secure for more info
#
ALL-y	+= u-boot-spl_HS_ISSW
ALL-y += u-boot-spl_HS_SPI_X-LOADER
ALL-y += u-boot-spl_HS_X-LOADER
ALL-y += u-boot-spl_HS_MLO
ALL-y += u-boot-spl_HS_2ND
else
ALL-y	+= MLO
ALL-y += MLO.byteswap
endif
else
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
ALL-$(CONFIG_QSPI_BOOT) += u-boot_HS_XIP_X-LOADER
ALL-$(CONFIG_SPL_LOAD_FIT) += u-boot_HS.img
endif
ALL-y	+= u-boot.img
endif
