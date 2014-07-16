#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_AT91SAM9_WATCHDOG) += at91sam9_wdt.o
obj-$(CONFIG_FTWDT010_WATCHDOG) += ftwdt010_wdt.o
ifneq (,$(filter $(SOC), mx31 mx35 mx5 mx6 vf610))
obj-y += imx_watchdog.o
endif
obj-$(CONFIG_TNETV107X_WATCHDOG) += tnetv107x_wdt.o
obj-$(CONFIG_S5P)               += s5p_wdt.o
obj-$(CONFIG_XILINX_TB_WATCHDOG) += xilinx_tb_wdt.o
obj-$(CONFIG_BFIN_WATCHDOG)  += bfin_wdt.o
obj-$(CONFIG_OMAP_WATCHDOG) += omap_wdt.o
obj-$(CONFIG_DESIGNWARE_WATCHDOG) += designware_wdt.o
