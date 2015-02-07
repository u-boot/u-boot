#
# (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
#
# Based on some other Makefile
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
obj-y	+= timer.o
obj-y	+= board.o
obj-y	+= clock.o
obj-y	+= cpu_info.o
obj-y	+= dram_helpers.o
obj-y	+= pinmux.o
obj-y	+= usbc.o
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
obj-$(CONFIG_MACH_SUN8I)	+= clock_sun6i.o
obj-$(CONFIG_MACH_SUN9I)	+= clock_sun9i.o

ifndef CONFIG_SPL_BUILD
ifdef CONFIG_ARMV7_PSCI
obj-y	+= psci.o
endif
endif

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_MACH_SUN4I)	+= dram_sun4i.o
obj-$(CONFIG_MACH_SUN5I)	+= dram_sun4i.o
obj-$(CONFIG_MACH_SUN6I)	+= dram_sun6i.o
obj-$(CONFIG_MACH_SUN7I)	+= dram_sun4i.o
obj-$(CONFIG_MACH_SUN8I)	+= dram_sun8i.o
obj-y	+= fel_utils.o
endif
