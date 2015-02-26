#
# SPDX-License-Identifier:	GPL-2.0+
#

# Machine directory name.  This list is sorted alphanumerically
# by CONFIG_* macro name.
machine-$(CONFIG_ARCH_AT91)		+= at91
machine-$(CONFIG_ARCH_DAVINCI)		+= davinci
machine-$(CONFIG_ARCH_HIGHBANK)		+= highbank
machine-$(CONFIG_ARCH_KEYSTONE)		+= keystone
# TODO: rename CONFIG_KIRKWOOD -> CONFIG_ARCH_KIRKWOOD
machine-$(CONFIG_KIRKWOOD)		+= kirkwood
# TODO: rename CONFIG_TEGRA -> CONFIG_ARCH_TEGRA
machine-$(CONFIG_ARCH_NOMADIK)		+= nomadik
# TODO: rename CONFIG_ORION5X -> CONFIG_ARCH_ORION5X
machine-$(CONFIG_ORION5X)		+= orion5x
machine-$(CONFIG_TEGRA)			+= tegra
machine-$(CONFIG_ARCH_UNIPHIER)		+= uniphier
machine-$(CONFIG_ARCH_VERSATILE)	+= versatile

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
