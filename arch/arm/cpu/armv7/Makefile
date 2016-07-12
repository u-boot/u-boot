#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	:= start.o

obj-y	+= cache_v7.o cache_v7_asm.o

obj-y	+= cpu.o cp15.o
obj-y	+= syslib.o

ifneq ($(CONFIG_AM43XX)$(CONFIG_AM33XX)$(CONFIG_OMAP44XX)$(CONFIG_OMAP54XX)$(CONFIG_TEGRA)$(CONFIG_MX6)$(CONFIG_MX7)$(CONFIG_TI81XX)$(CONFIG_AT91FAMILY)$(CONFIG_SUNXI)$(CONFIG_ARCH_SOCFPGA),)
ifneq ($(CONFIG_SKIP_LOWLEVEL_INIT),y)
obj-y	+= lowlevel_init.o
endif
endif

obj-$(CONFIG_ARMV7_NONSEC)	+= nonsec_virt.o virt-v7.o virt-dt.o
obj-$(CONFIG_ARMV7_PSCI)	+= psci.o

obj-$(CONFIG_IPROC) += iproc-common/
obj-$(CONFIG_KONA) += kona-common/
obj-$(CONFIG_OMAP_COMMON) += omap-common/
obj-$(CONFIG_SYS_ARCH_TIMER) += arch_timer.o

ifneq (,$(filter s5pc1xx exynos,$(SOC)))
obj-y += s5p-common/
endif

obj-$(if $(filter am33xx,$(SOC)),y) += am33xx/
obj-$(if $(filter bcm235xx,$(SOC)),y) += bcm235xx/
obj-$(if $(filter bcm281xx,$(SOC)),y) += bcm281xx/
obj-$(if $(filter bcmcygnus,$(SOC)),y) += bcmcygnus/
obj-$(if $(filter bcmnsp,$(SOC)),y) += bcmnsp/
obj-$(if $(filter ls102xa,$(SOC)),y) += ls102xa/
obj-$(if $(filter mx5,$(SOC)),y) += mx5/
obj-$(CONFIG_MX6) += mx6/
obj-$(CONFIG_MX7) += mx7/
obj-$(CONFIG_OMAP34XX) += omap3/
obj-$(CONFIG_OMAP44XX) += omap4/
obj-$(CONFIG_OMAP54XX) += omap5/
obj-$(CONFIG_RMOBILE) += rmobile/
obj-$(if $(filter stv0991,$(SOC)),y) += stv0991/
obj-$(CONFIG_ARCH_SUNXI) += sunxi/
obj-$(CONFIG_VF610) += vf610/
