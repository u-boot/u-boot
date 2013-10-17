#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	:= start.o

obj-y	+= cache_v7.o

obj-y	+= cpu.o
obj-y	+= syslib.o

ifneq ($(CONFIG_AM43XX)$(CONFIG_AM33XX)$(CONFIG_OMAP44XX)$(CONFIG_OMAP54XX)$(CONFIG_TEGRA)$(CONFIG_MX6)$(CONFIG_TI81XX),)
obj-y	+= lowlevel_init.o
endif

ifneq ($(CONFIG_ARMV7_NONSEC)$(CONFIG_ARMV7_VIRT),)
obj-y	+= nonsec_virt.o
obj-y	+= virt-v7.o
endif
