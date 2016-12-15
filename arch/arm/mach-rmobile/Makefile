#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y = cpu_info.o
obj-y += emac.o

obj-$(CONFIG_DISPLAY_BOARDINFO) += board.o
obj-$(CONFIG_GLOBAL_TIMER) += timer.o
obj-$(CONFIG_R8A7740) += lowlevel_init.o cpu_info-r8a7740.o pfc-r8a7740.o
obj-$(CONFIG_R8A7790) += lowlevel_init_ca15.o cpu_info-rcar.o pfc-r8a7790.o
obj-$(CONFIG_R8A7791) += lowlevel_init_ca15.o cpu_info-rcar.o pfc-r8a7791.o
obj-$(CONFIG_R8A7792) += lowlevel_init_ca15.o cpu_info-rcar.o pfc-r8a7792.o
obj-$(CONFIG_R8A7793) += lowlevel_init_ca15.o cpu_info-rcar.o pfc-r8a7793.o
obj-$(CONFIG_R8A7794) += lowlevel_init_ca15.o cpu_info-rcar.o pfc-r8a7794.o
obj-$(CONFIG_R8A7795) += lowlevel_init_gen3.o cpu_info-rcar.o pfc-r8a7795.o memmap-r8a7795.o
obj-$(CONFIG_SH73A0) += lowlevel_init.o cpu_info-sh73a0.o pfc-sh73a0.o
obj-$(CONFIG_TMU_TIMER) += ../../sh/lib/time.o
