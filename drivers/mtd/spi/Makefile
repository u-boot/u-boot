# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_$(PHASE_)DM_SPI_FLASH) += sf-uclass.o
spi-nor-y := sf_probe.o spi-nor-ids.o

ifdef CONFIG_XPL_BUILD
obj-$(CONFIG_SPL_SPI_BOOT)	+= fsl_espi_spl.o
endif

ifeq ($(CONFIG_$(PHASE_)SPI_FLASH_TINY),y)
spi-nor-y += spi-nor-tiny.o
else
spi-nor-y += spi-nor-core.o
spi-nor-$(CONFIG_SPI_FLASH_SFDP_SUPPORT) += sfdp.o
endif

spi-nor-$(CONFIG_SPI_FLASH_ATMEL) += atmel.o
spi-nor-$(CONFIG_SPI_FLASH_DOSILICON) += dosilicon.o
spi-nor-$(CONFIG_SPI_FLASH_EON) += eon.o
spi-nor-$(CONFIG_SPI_FRAM_FUJITSU) += fujitsu.o
spi-nor-$(CONFIG_SPI_FLASH_GIGADEVICE) += gigadevice.o
spi-nor-$(CONFIG_SPI_FLASH_ISSI) += issi.o
spi-nor-$(CONFIG_SPI_FLASH_MACRONIX) += macronix.o
spi-nor-$(CONFIG_SPI_FLASH_PUYA) += puya.o
spi-nor-$(CONFIG_SPI_FLASH_SILICONKAISER) += siliconkaiser.o
spi-nor-$(CONFIG_SPI_FLASH_STMICRO) += micron-st.o
spi-nor-$(CONFIG_SPI_FLASH_SPANSION) += spansion.o
spi-nor-$(CONFIG_SPI_FLASH_SST) += sst.o
spi-nor-$(CONFIG_SPI_FLASH_WINBOND) += winbond.o
spi-nor-$(CONFIG_SPI_FLASH_XMC) += xmc.o
spi-nor-$(CONFIG_SPI_FLASH_XTX) += xtx.o
spi-nor-$(CONFIG_SPI_FLASH_ZBIT) += zbit.o

obj-$(CONFIG_SPI_FLASH) += spi-nor.o
obj-$(CONFIG_SPI_FLASH_DATAFLASH) += sf_dataflash.o
obj-$(CONFIG_$(PHASE_)SPI_FLASH_MTD) += sf_mtd.o
obj-$(CONFIG_SPI_FLASH_SANDBOX) += sandbox.o
obj-$(CONFIG_$(PHASE_)BOOTDEV_SPI_FLASH) += sf_bootdev.o
