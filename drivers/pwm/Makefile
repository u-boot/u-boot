#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2001
# Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#ccflags-y += -DDEBUG

obj-$(CONFIG_DM_PWM) += pwm-uclass.o
obj-$(CONFIG_PWM_ROCKCHIP) += rk_pwm.o
obj-$(CONFIG_PWM_IMX) += pwm-imx.o pwm-imx-util.o
ifdef CONFIG_DM_PWM
obj-$(CONFIG_PWM_TEGRA) += tegra_pwm.o
obj-$(CONFIG_PWM_EXYNOS) += exynos_pwm.o
endif
