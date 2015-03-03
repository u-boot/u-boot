#
# Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_DM_GPIO)		+= gpio-uclass.o
endif
/* TODO(sjg@chromium.org): Only tegra supports driver model in SPL */
ifdef CONFIG_TEGRA_GPIO
obj-$(CONFIG_DM_GPIO)		+= gpio-uclass.o
endif

obj-$(CONFIG_AT91_GPIO)	+= at91_gpio.o
obj-$(CONFIG_INTEL_ICH6_GPIO)	+= intel_ich6_gpio.o
obj-$(CONFIG_KIRKWOOD_GPIO)	+= kw_gpio.o
obj-$(CONFIG_KONA_GPIO)	+= kona_gpio.o
obj-$(CONFIG_MARVELL_GPIO)	+= mvgpio.o
obj-$(CONFIG_MARVELL_MFP)	+= mvmfp.o
obj-$(CONFIG_MXC_GPIO)	+= mxc_gpio.o
obj-$(CONFIG_MXS_GPIO)	+= mxs_gpio.o
obj-$(CONFIG_PCA953X)		+= pca953x.o
obj-$(CONFIG_PCA9698)		+= pca9698.o
obj-$(CONFIG_S5P)		+= s5p_gpio.o
obj-$(CONFIG_SANDBOX_GPIO)	+= sandbox.o
obj-$(CONFIG_SPEAR_GPIO)	+= spear_gpio.o
obj-$(CONFIG_TEGRA_GPIO)	+= tegra_gpio.o
obj-$(CONFIG_DA8XX_GPIO)	+= da8xx_gpio.o
obj-$(CONFIG_DM644X_GPIO)	+= da8xx_gpio.o
obj-$(CONFIG_ALTERA_PIO)	+= altera_pio.o
obj-$(CONFIG_MPC83XX_GPIO)	+= mpc83xx_gpio.o
obj-$(CONFIG_SH_GPIO_PFC)	+= sh_pfc.o
obj-$(CONFIG_OMAP_GPIO)	+= omap_gpio.o
obj-$(CONFIG_DB8500_GPIO)	+= db8500_gpio.o
obj-$(CONFIG_BCM2835_GPIO)	+= bcm2835_gpio.o
obj-$(CONFIG_S3C2440_GPIO)	+= s3c2440_gpio.o
obj-$(CONFIG_XILINX_GPIO)	+= xilinx_gpio.o
obj-$(CONFIG_ADI_GPIO2)	+= adi_gpio2.o
obj-$(CONFIG_TCA642X)		+= tca642x.o
oby-$(CONFIG_SX151X)		+= sx151x.o
obj-$(CONFIG_SUNXI_GPIO)	+= sunxi_gpio.o
