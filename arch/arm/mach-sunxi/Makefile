#
# (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
#
# Based on some other Makefile
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= board.o
obj-y	+= clock.o
obj-y	+= cpu_info.o
obj-y	+= dram_helpers.o
obj-y	+= pinmux.o
ifndef CONFIG_MACH_SUN9I
obj-y	+= usb_phy.o
endif
obj-$(CONFIG_MACH_SUN6I)	+= prcm.o
obj-$(CONFIG_MACH_SUN8I)	+= prcm.o
obj-$(CONFIG_MACH_SUN9I)	+= prcm.o
obj-$(CONFIG_MACH_SUN6I)	+= p2wi.o
obj-$(CONFIG_MACH_SUN8I)	+= rsb.o
obj-$(CONFIG_MACH_SUN9I)	+= rsb.o
obj-$(CONFIG_MACH_SUN4I)	+= clock_sun4i.o
obj-$(CONFIG_MACH_SUN5I)	+= clock_sun4i.o
obj-$(CONFIG_MACH_SUN6I)	+= clock_sun6i.o
obj-$(CONFIG_MACH_SUN7I)	+= clock_sun4i.o
obj-$(CONFIG_MACH_SUN50I)	+= clock_sun6i.o
ifdef CONFIG_MACH_SUN8I_A83T
obj-y				+= clock_sun8i_a83t.o
else
obj-$(CONFIG_MACH_SUN8I)	+= clock_sun6i.o
endif
obj-$(CONFIG_MACH_SUN9I)	+= clock_sun9i.o gtbus_sun9i.o

obj-$(CONFIG_AXP152_POWER)	+= pmic_bus.o
obj-$(CONFIG_AXP209_POWER)	+= pmic_bus.o
obj-$(CONFIG_AXP221_POWER)	+= pmic_bus.o
obj-$(CONFIG_AXP809_POWER)	+= pmic_bus.o
obj-$(CONFIG_AXP818_POWER)	+= pmic_bus.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_MACH_SUN4I)	+= dram_sun4i.o
obj-$(CONFIG_MACH_SUN5I)	+= dram_sun4i.o
obj-$(CONFIG_MACH_SUN6I)	+= dram_sun6i.o
obj-$(CONFIG_MACH_SUN7I)	+= dram_sun4i.o
obj-$(CONFIG_MACH_SUN8I_A23)	+= dram_sun8i_a23.o
obj-$(CONFIG_MACH_SUN8I_A33)	+= dram_sun8i_a33.o
obj-$(CONFIG_MACH_SUN8I_A83T)	+= dram_sun8i_a83t.o
obj-$(CONFIG_MACH_SUNXI_H3_H5)	+= dram_sun8i_h3.o
obj-$(CONFIG_MACH_SUN8I_R40)	+= dram_sun8i_h3.o
obj-$(CONFIG_MACH_SUN9I)	+= dram_sun9i.o
obj-$(CONFIG_MACH_SUN50I)	+= dram_sun8i_h3.o
endif
