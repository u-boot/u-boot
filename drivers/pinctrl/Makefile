#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y					+= pinctrl-uclass.o
obj-$(CONFIG_$(SPL_)PINCTRL_GENERIC)	+= pinctrl-generic.o

obj-y					+= nxp/
obj-$(CONFIG_ARCH_ATH79) += ath79/
obj-$(CONFIG_ARCH_ROCKCHIP) += rockchip/
obj-$(CONFIG_PINCTRL_SANDBOX)	+= pinctrl-sandbox.o

obj-$(CONFIG_PINCTRL_UNIPHIER)	+= uniphier/
obj-$(CONFIG_PIC32_PINCTRL)	+= pinctrl_pic32.o
obj-$(CONFIG_PINCTRL_EXYNOS)	+= exynos/
