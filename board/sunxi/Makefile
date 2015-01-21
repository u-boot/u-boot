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
obj-$(CONFIG_SUNXI_GMAC)	+= gmac.o
obj-$(CONFIG_SUNXI_AHCI)	+= ahci.o
obj-$(CONFIG_MACH_SUN4I)	+= dram_sun4i_auto.o
obj-$(CONFIG_MACH_SUN7I)	+= dram_sun5i_auto.o
obj-$(CONFIG_TARGET_A10S_OLINUXINO_M)	+= dram_a10s_olinuxino_m.o
obj-$(CONFIG_TARGET_A13_OLINUXINO)	+= dram_a13_olinuxino.o
obj-$(CONFIG_TARGET_A13_OLINUXINOM)	+= dram_a13_oli_micro.o
# This is not a typo, uses the same mem settings as the a10s-olinuxino-m
obj-$(CONFIG_TARGET_AUXTEK_T004)	+= dram_a10s_olinuxino_m.o
obj-$(CONFIG_TARGET_MK802_A10S)		+= dram_sun5i_auto.o
obj-$(CONFIG_TARGET_R7DONGLE)		+= dram_r7dongle.o
