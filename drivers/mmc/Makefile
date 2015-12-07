#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_DM_MMC) += mmc-uclass.o

obj-$(CONFIG_ARM_PL180_MMCI) += arm_pl180_mmci.o
obj-$(CONFIG_BCM2835_SDHCI) += bcm2835_sdhci.o
obj-$(CONFIG_BFIN_SDH) += bfin_sdh.o
obj-$(CONFIG_DAVINCI_MMC) += davinci_mmc.o
obj-$(CONFIG_DWMMC) += dw_mmc.o
obj-$(CONFIG_EXYNOS_DWMMC) += exynos_dw_mmc.o
obj-$(CONFIG_HIKEY_DWMMC) += hi6220_dw_mmc.o
obj-$(CONFIG_FSL_ESDHC) += fsl_esdhc.o
obj-$(CONFIG_FTSDC010) += ftsdc010_mci.o
obj-$(CONFIG_FTSDC021) += ftsdc021_sdhci.o
obj-$(CONFIG_GENERIC_MMC) += mmc.o
obj-$(CONFIG_GENERIC_ATMEL_MCI) += gen_atmel_mci.o
obj-$(CONFIG_KONA_SDHCI) += kona_sdhci.o
obj-$(CONFIG_MMC_SPI) += mmc_spi.o
obj-$(CONFIG_MMC_SUNXI) += sunxi_mmc.o
obj-$(CONFIG_MV_SDHCI) += mv_sdhci.o
obj-$(CONFIG_MVEBU_MMC) += mvebu_mmc.o
obj-$(CONFIG_MXC_MMC) += mxcmmc.o
obj-$(CONFIG_MXS_MMC) += mxsmmc.o
obj-$(CONFIG_OMAP_HSMMC) += omap_hsmmc.o
obj-$(CONFIG_X86) += pci_mmc.o
obj-$(CONFIG_PXA_MMC_GENERIC) += pxa_mmc_gen.o
obj-$(CONFIG_ROCKCHIP_DWMMC) += rockchip_dw_mmc.o
obj-$(CONFIG_SUPPORT_EMMC_RPMB) += rpmb.o
obj-$(CONFIG_S3C_SDI) += s3c_sdi.o
obj-$(CONFIG_S5P_SDHCI) += s5p_sdhci.o
obj-$(CONFIG_SANDBOX) += sandbox_mmc.o
obj-$(CONFIG_SDHCI) += sdhci.o
obj-$(CONFIG_SH_MMCIF) += sh_mmcif.o
obj-$(CONFIG_SH_SDHI) += sh_sdhi.o
obj-$(CONFIG_SOCFPGA_DWMMC) += socfpga_dw_mmc.o
obj-$(CONFIG_SPEAR_SDHCI) += spear_sdhci.o
obj-$(CONFIG_TEGRA_MMC) += tegra_mmc.o
obj-$(CONFIG_ZYNQ_SDHCI) += zynq_sdhci.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_MMC_BOOT) += fsl_esdhc_spl.o
else
obj-$(CONFIG_GENERIC_MMC) += mmc_write.o
endif

