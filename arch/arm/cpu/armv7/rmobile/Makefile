#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y = lowlevel_init.o
obj-y += cpu_info.o
obj-y += emac.o

obj-$(CONFIG_DISPLAY_BOARDINFO) += board.o
obj-$(CONFIG_GLOBAL_TIMER) += timer.o
obj-$(CONFIG_R8A7740) += cpu_info-r8a7740.o
obj-$(CONFIG_R8A7740) += pfc-r8a7740.o
obj-$(CONFIG_SH73A0) += cpu_info-sh73a0.o
obj-$(CONFIG_SH73A0) += pfc-sh73a0.o
obj-$(CONFIG_TMU_TIMER) += ../../../../sh/lib/time.o
