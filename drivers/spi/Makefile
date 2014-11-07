#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# There are many options which enable SPI, so make this library available
ifdef CONFIG_DM_SPI
obj-y += spi-uclass.o
obj-$(CONFIG_SANDBOX) += spi-emul-uclass.o
obj-$(CONFIG_SOFT_SPI) += soft_spi.o
else
obj-y += spi.o
obj-$(CONFIG_SOFT_SPI) += soft_spi_legacy.o
endif

obj-$(CONFIG_EP93XX_SPI) += ep93xx_spi.o
obj-$(CONFIG_ALTERA_SPI) += altera_spi.o
obj-$(CONFIG_ANDES_SPI) += andes_spi.o
obj-$(CONFIG_ARMADA100_SPI) += armada100_spi.o
obj-$(CONFIG_ATMEL_DATAFLASH_SPI) += atmel_dataflash_spi.o
obj-$(CONFIG_ATMEL_SPI) += atmel_spi.o
obj-$(CONFIG_BFIN_SPI) += bfin_spi.o
obj-$(CONFIG_BFIN_SPI6XX) += bfin_spi6xx.o
obj-$(CONFIG_CADENCE_QSPI) += cadence_qspi.o cadence_qspi_apb.o
obj-$(CONFIG_CF_SPI) += cf_spi.o
obj-$(CONFIG_CF_QSPI) += cf_qspi.o
obj-$(CONFIG_DAVINCI_SPI) += davinci_spi.o
obj-$(CONFIG_DESIGNWARE_SPI) += designware_spi.o
obj-$(CONFIG_EXYNOS_SPI) += exynos_spi.o
obj-$(CONFIG_FTSSP010_SPI) += ftssp010_spi.o
obj-$(CONFIG_ICH_SPI) +=  ich.o
obj-$(CONFIG_KIRKWOOD_SPI) += kirkwood_spi.o
obj-$(CONFIG_MPC52XX_SPI) += mpc52xx_spi.o
obj-$(CONFIG_MPC8XXX_SPI) += mpc8xxx_spi.o
obj-$(CONFIG_MXC_SPI) += mxc_spi.o
obj-$(CONFIG_MXS_SPI) += mxs_spi.o
obj-$(CONFIG_OC_TINY_SPI) += oc_tiny_spi.o
obj-$(CONFIG_OMAP3_SPI) += omap3_spi.o
obj-$(CONFIG_SANDBOX_SPI) += sandbox_spi.o
obj-$(CONFIG_SH_SPI) += sh_spi.o
obj-$(CONFIG_SH_QSPI) += sh_qspi.o
obj-$(CONFIG_FSL_ESPI) += fsl_espi.o
obj-$(CONFIG_TEGRA20_SFLASH) += tegra20_sflash.o
obj-$(CONFIG_TEGRA20_SLINK) += tegra20_slink.o
obj-$(CONFIG_TEGRA114_SPI) += tegra114_spi.o
obj-$(CONFIG_TI_QSPI) += ti_qspi.o
obj-$(CONFIG_XILINX_SPI) += xilinx_spi.o
obj-$(CONFIG_ZYNQ_SPI) += zynq_spi.o
obj-$(CONFIG_FSL_QSPI) += fsl_qspi.o
