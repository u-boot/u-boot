#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libwatchdog.o

COBJS-$(CONFIG_AT91SAM9_WATCHDOG) += at91sam9_wdt.o
COBJS-$(CONFIG_FTWDT010_WATCHDOG) += ftwdt010_wdt.o
ifneq (,$(filter $(SOC), mx31 mx35 mx5 mx6 vf610))
COBJS-y += imx_watchdog.o
endif
COBJS-$(CONFIG_TNETV107X_WATCHDOG) += tnetv107x_wdt.o
COBJS-$(CONFIG_S5P)               += s5p_wdt.o
COBJS-$(CONFIG_XILINX_TB_WATCHDOG) += xilinx_tb_wdt.o
COBJS-$(CONFIG_BFIN_WATCHDOG)  += bfin_wdt.o
COBJS-$(CONFIG_OMAP_WATCHDOG) += omap_wdt.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
