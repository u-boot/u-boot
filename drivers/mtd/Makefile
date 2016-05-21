#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifneq (,$(findstring y,$(CONFIG_MTD_DEVICE)$(CONFIG_CMD_NAND)$(CONFIG_CMD_ONENAND)$(CONFIG_CMD_SF)))
obj-y += mtdcore.o mtd_uboot.o
endif
obj-$(CONFIG_MTD) += mtd-uclass.o
obj-$(CONFIG_MTD_PARTITIONS) += mtdpart.o
obj-$(CONFIG_MTD_CONCAT) += mtdconcat.o
obj-$(CONFIG_ALTERA_QSPI) += altera_qspi.o
obj-$(CONFIG_HAS_DATAFLASH) += at45.o
obj-$(CONFIG_FLASH_CFI_DRIVER) += cfi_flash.o
obj-$(CONFIG_FLASH_CFI_MTD) += cfi_mtd.o
obj-$(CONFIG_HAS_DATAFLASH) += dataflash.o
obj-$(CONFIG_FTSMC020) += ftsmc020.o
obj-$(CONFIG_FLASH_CFI_LEGACY) += jedec_flash.o
obj-$(CONFIG_MW_EEPROM) += mw_eeprom.o
obj-$(CONFIG_FLASH_PIC32) += pic32_flash.o
obj-$(CONFIG_ST_SMI) += st_smi.o
obj-$(CONFIG_STM32_FLASH) += stm32_flash.o
