#
# (C) Copyright 2013-2014
# NVIDIA Corporation <www.nvidia.com>
#
# SPDX-License-Identifier:     GPL-2.0+
#

obj-$(CONFIG_XPL_BUILD) += cpu.o
obj-$(CONFIG_$(XPL_)CMD_EBTUPDATE) += bct.o

obj-y	+= clock.o
obj-y	+= pmc.o
obj-y	+= xusb-padctl.o
obj-y	+= ../xusb-padctl-common.o

ifndef CONFIG_XPL_BUILD
obj-$(CONFIG_ARMV7_NONSEC) += psci.o
endif
