#
# (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
#
# Based on some other board Makefile
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
obj-y	+= board.o
obj-$(CONFIG_SUN7I_GMAC)	+= gmac.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_SUNXI_AHCI)	+= ahci.o
endif
obj-$(CONFIG_MACH_SUN4I)	+= dram_sun4i_auto.o
obj-$(CONFIG_MACH_SUN5I)	+= dram_sun5i_auto.o
obj-$(CONFIG_MACH_SUN7I)	+= dram_sun5i_auto.o
