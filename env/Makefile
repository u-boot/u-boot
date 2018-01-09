#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += common.o env.o

ifndef CONFIG_SPL_BUILD
obj-y += attr.o
obj-y += callback.o
obj-y += flags.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += eeprom.o
extra-$(CONFIG_ENV_IS_EMBEDDED) += embedded.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += embedded.o
extra-$(CONFIG_ENV_IS_IN_FLASH) += embedded.o
obj-$(CONFIG_ENV_IS_IN_NVRAM) += embedded.o
obj-$(CONFIG_ENV_IS_IN_FLASH) += flash.o
obj-$(CONFIG_ENV_IS_IN_MMC) += mmc.o
obj-$(CONFIG_ENV_IS_IN_FAT) += fat.o
obj-$(CONFIG_ENV_IS_IN_EXT4) += ext4.o
obj-$(CONFIG_ENV_IS_IN_NAND) += nand.o
obj-$(CONFIG_ENV_IS_IN_NVRAM) += nvram.o
obj-$(CONFIG_ENV_IS_IN_ONENAND) += onenand.o
obj-$(CONFIG_ENV_IS_IN_SATA) += sata.o
obj-$(CONFIG_ENV_IS_IN_SPI_FLASH) += sf.o
obj-$(CONFIG_ENV_IS_IN_REMOTE) += remote.o
obj-$(CONFIG_ENV_IS_IN_UBI) += ubi.o
obj-$(CONFIG_ENV_IS_NOWHERE) += nowhere.o
endif

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ENV_IS_IN_FLASH) += flash.o
# environment
ifdef CONFIG_TPL_BUILD
obj-$(CONFIG_TPL_ENV_SUPPORT) += attr.o
obj-$(CONFIG_TPL_ENV_SUPPORT) += flags.o
obj-$(CONFIG_TPL_ENV_SUPPORT) += callback.o
else
obj-$(CONFIG_SPL_ENV_SUPPORT) += attr.o
obj-$(CONFIG_SPL_ENV_SUPPORT) += flags.o
obj-$(CONFIG_SPL_ENV_SUPPORT) += callback.o
endif
ifneq ($(CONFIG_TPL_ENV_SUPPORT)$(CONFIG_SPL_ENV_SUPPORT),)
obj-$(CONFIG_ENV_IS_NOWHERE) += nowhere.o
obj-$(CONFIG_ENV_IS_IN_MMC) += mmc.o
obj-$(CONFIG_ENV_IS_IN_FAT) += fat.o
obj-$(CONFIG_ENV_IS_IN_EXT4) += ext4.o
obj-$(CONFIG_ENV_IS_IN_NAND) += nand.o
obj-$(CONFIG_ENV_IS_IN_SPI_FLASH) += sf.o
obj-$(CONFIG_ENV_IS_IN_FLASH) += flash.o
endif
endif

CFLAGS_embedded.o := -Wa,--no-warn -DENV_CRC=$(shell tools/envcrc 2>/dev/null)
