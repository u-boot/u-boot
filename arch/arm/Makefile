#
# SPDX-License-Identifier:	GPL-2.0+
#

# Machine directory name.  This list is sorted alphanumerically
# by CONFIG_* macro name.
machine-$(CONFIG_ARCH_AT91)		+= at91
machine-$(CONFIG_ARCH_DAVINCI)		+= davinci
# TODO: rename CONFIG_TEGRA -> CONFIG_ARCH_TEGRA
machine-$(CONFIG_TEGRA)			+= tegra

machdirs := $(patsubst %,arch/arm/mach-%/,$(machine-y))

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
ifneq (,$(CONFIG_MX23)$(CONFIG_MX35)$(filter $(SOC), mx25 mx27 mx5 mx6 mx31 mx35))
libs-y += arch/arm/imx-common/
endif
else
ifneq (,$(filter $(SOC), mx25 mx27 mx5 mx6 mx31 mx35 mxs vf610))
libs-y += arch/arm/imx-common/
endif
endif

ifneq (,$(filter $(SOC), armada-xp kirkwood))
libs-y += arch/arm/mvebu-common/
endif

# deprecated
-include $(machdirs)/config.mk
