#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_SPI_LOAD)	+= spi_spl_load.o
obj-$(CONFIG_SPL_SPI_BOOT)	+= fsl_espi_spl.o
endif

obj-$(CONFIG_CMD_SF) += sf.o
obj-$(CONFIG_SPI_FLASH) += sf_params.o sf_probe.o sf_ops.o
obj-$(CONFIG_SPI_FRAM_RAMTRON) += ramtron.o
obj-$(CONFIG_SPI_FLASH_SANDBOX) += sandbox.o
obj-$(CONFIG_SPI_M95XXX) += eeprom_m95xxx.o
