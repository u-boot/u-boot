#
# Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier:      GPL-2.0+

include  $(srctree)/arch/arm/mach-omap2/config_secure.mk

ifdef CONFIG_XPL_BUILD
ifeq ($(CONFIG_TI_SECURE_DEVICE),y) # Refer to README.ti-secure for more info
# On DRA7xx/AM57xx:
#
# For booting spl from SD/MMC/eMMC use
# u-boot-spl_HS_MLO
#
# For booting spl over UART or USB use
# u-boot-spl_HS_ULO
#
# For booting spl from QSPI or NOR use
# u-boot-spl_HS_X-LOADER
ifeq ($(CONFIG_OMAP54XX),y)
INPUTS-y += u-boot-spl_HS_MLO
INPUTS-y += u-boot-spl_HS_ULO
INPUTS-y += u-boot-spl_HS_X-LOADER
endif
# On AM43XX:
#
# For booting spl from SPI flash use
# u-boot-spl_HS_SPI_X-LOADER
#
# For booting spl from all other media use
# u-boot-spl_HS_ISSW
ifeq ($(CONFIG_AM43XX),y)
INPUTS-y += u-boot-spl_HS_SPI_X-LOADER
INPUTS-y += u-boot-spl_HS_ISSW
endif
# On AM33XX:
#
# For booting spl from SPI flash use
# u-boot-spl_HS_SPI_X-LOADER
#
# For booting spl from NAND flash or raw SD/MMC/eMMC use
# u-boot-spl_HS_X-LOADER
#
# For booting spl from a filesystem on SD/MMC/eMMC use
# u-boot-spl_HS_MLO
#
# For booting spl over UART, USB, or Ethernet use
# u-boot-spl_HS_2ND
ifeq ($(CONFIG_AM33XX),y)
INPUTS-y += u-boot-spl_HS_SPI_X-LOADER
INPUTS-y += u-boot-spl_HS_X-LOADER
INPUTS-y += u-boot-spl_HS_MLO
INPUTS-y += u-boot-spl_HS_2ND
endif
else
INPUTS-y += MLO
ifeq ($(CONFIG_AM33XX),y)
INPUTS-y += MLO.byteswap
endif
endif
else
ifeq ($(CONFIG_TI_SECURE_DEVICE),y)
INPUTS-$(CONFIG_QSPI_BOOT) += u-boot_HS_XIP_X-LOADER
INPUTS-$(CONFIG_SPL_LOAD_FIT) += u-boot_HS.img
endif
INPUTS-y += u-boot.img
endif
