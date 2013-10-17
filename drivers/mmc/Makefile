#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libmmc.o


COBJS-$(CONFIG_BFIN_SDH) += bfin_sdh.o
COBJS-$(CONFIG_DAVINCI_MMC) += davinci_mmc.o
COBJS-$(CONFIG_FSL_ESDHC) += fsl_esdhc.o
COBJS-$(CONFIG_FTSDC010) += ftsdc010_mci.o
COBJS-$(CONFIG_GENERIC_MMC) += mmc.o
COBJS-$(CONFIG_GENERIC_ATMEL_MCI) += gen_atmel_mci.o
COBJS-$(CONFIG_MMC_SPI) += mmc_spi.o
COBJS-$(CONFIG_ARM_PL180_MMCI) += arm_pl180_mmci.o
COBJS-$(CONFIG_MV_SDHCI) += mv_sdhci.o
COBJS-$(CONFIG_MXC_MMC) += mxcmmc.o
COBJS-$(CONFIG_MXS_MMC) += mxsmmc.o
COBJS-$(CONFIG_OMAP_HSMMC) += omap_hsmmc.o
COBJS-$(CONFIG_PXA_MMC_GENERIC) += pxa_mmc_gen.o
COBJS-$(CONFIG_SDHCI) += sdhci.o
COBJS-$(CONFIG_BCM2835_SDHCI) += bcm2835_sdhci.o
COBJS-$(CONFIG_S5P_SDHCI) += s5p_sdhci.o
COBJS-$(CONFIG_SH_MMCIF) += sh_mmcif.o
COBJS-$(CONFIG_SPEAR_SDHCI) += spear_sdhci.o
COBJS-$(CONFIG_TEGRA_MMC) += tegra_mmc.o
COBJS-$(CONFIG_DWMMC) += dw_mmc.o
COBJS-$(CONFIG_EXYNOS_DWMMC) += exynos_dw_mmc.o
COBJS-$(CONFIG_ZYNQ_SDHCI) += zynq_sdhci.o
ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_MMC_BOOT) += fsl_esdhc_spl.o
else
COBJS-$(CONFIG_GENERIC_MMC) += mmc_write.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
