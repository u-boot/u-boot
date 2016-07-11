#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TEGRA),yy)
CONFIG_CPU_V7=
CONFIG_CPU_ARM720T=y
endif

# This selects which instruction set is used.
arch-$(CONFIG_CPU_ARM720T)	=-march=armv4
arch-$(CONFIG_CPU_ARM920T)	=-march=armv4t
arch-$(CONFIG_CPU_ARM926EJS)	=-march=armv5te
arch-$(CONFIG_CPU_ARM946ES)	=-march=armv4
arch-$(CONFIG_CPU_SA1100)	=-march=armv4
arch-$(CONFIG_CPU_PXA)		=
arch-$(CONFIG_CPU_ARM1136)	=-march=armv5
arch-$(CONFIG_CPU_ARM1176)	=-march=armv5t
arch-$(CONFIG_CPU_V7)		=$(call cc-option, -march=armv7-a, \
				 $(call cc-option, -march=armv7, -march=armv5))
arch-$(CONFIG_ARM64)		=-march=armv8-a

# Evaluate arch cc-option calls now
arch-y := $(arch-y)

# This selects how we optimise for the processor.
tune-$(CONFIG_CPU_ARM720T)	=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM920T)	=
tune-$(CONFIG_CPU_ARM926EJS)	=
tune-$(CONFIG_CPU_ARM946ES)	=
tune-$(CONFIG_CPU_SA1100)	=-mtune=strongarm1100
tune-$(CONFIG_CPU_PXA)		=-mcpu=xscale
tune-$(CONFIG_CPU_ARM1136)	=
tune-$(CONFIG_CPU_ARM1176)	=
tune-$(CONFIG_CPU_V7)		=
tune-$(CONFIG_ARM64)		=

# Evaluate tune cc-option calls now
tune-y := $(tune-y)

PLATFORM_CPPFLAGS += $(arch-y) $(tune-y)

# Machine directory name.  This list is sorted alphanumerically
# by CONFIG_* macro name.
machine-$(CONFIG_ARCH_AT91)		+= at91
machine-$(CONFIG_ARCH_BCM283X)		+= bcm283x
machine-$(CONFIG_ARCH_DAVINCI)		+= davinci
machine-$(CONFIG_ARCH_EXYNOS)		+= exynos
machine-$(CONFIG_ARCH_HIGHBANK)		+= highbank
machine-$(CONFIG_ARCH_KEYSTONE)		+= keystone
# TODO: rename CONFIG_KIRKWOOD -> CONFIG_ARCH_KIRKWOOD
machine-$(CONFIG_KIRKWOOD)		+= kirkwood
machine-$(CONFIG_ARCH_MVEBU)		+= mvebu
# TODO: rename CONFIG_TEGRA -> CONFIG_ARCH_TEGRA
# TODO: rename CONFIG_ORION5X -> CONFIG_ARCH_ORION5X
machine-$(CONFIG_ORION5X)		+= orion5x
machine-$(CONFIG_ARCH_S5PC1XX)		+= s5pc1xx
machine-$(CONFIG_ARCH_SUNXI)		+= sunxi
machine-$(CONFIG_ARCH_SNAPDRAGON)	+= snapdragon
machine-$(CONFIG_ARCH_SOCFPGA)		+= socfpga
machine-$(CONFIG_ARCH_ROCKCHIP)		+= rockchip
machine-$(CONFIG_STM32)			+= stm32
machine-$(CONFIG_TEGRA)			+= tegra
machine-$(CONFIG_ARCH_UNIPHIER)		+= uniphier
machine-$(CONFIG_ARCH_ZYNQ)		+= zynq

machdirs := $(patsubst %,arch/arm/mach-%/,$(machine-y))

PLATFORM_CPPFLAGS += $(patsubst %,-I$(srctree)/%include,$(machdirs))

libs-y += $(machdirs)

head-y := arch/arm/cpu/$(CPU)/start.o

ifeq ($(CONFIG_SPL_BUILD),y)
ifneq ($(CONFIG_SPL_START_S_PATH),)
head-y := $(CONFIG_SPL_START_S_PATH:"%"=%)/start.o
endif
endif

libs-y += arch/arm/cpu/$(CPU)/
libs-y += arch/arm/cpu/
libs-y += arch/arm/lib/

ifeq ($(CONFIG_SPL_BUILD),y)
ifneq (,$(CONFIG_MX23)$(CONFIG_MX28)$(CONFIG_MX35)$(filter $(SOC), mx25 mx27 mx5 mx6 mx7 mx31 mx35))
libs-y += arch/arm/imx-common/
endif
else
ifneq (,$(filter $(SOC), mx25 mx27 mx5 mx6 mx7 mx31 mx35 mxs vf610))
libs-y += arch/arm/imx-common/
endif
endif

ifneq (,$(filter $(SOC), kirkwood))
libs-y += arch/arm/mach-mvebu/
endif

# deprecated
-include $(machdirs)/config.mk
