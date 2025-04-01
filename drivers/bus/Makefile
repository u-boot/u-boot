# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the bus drivers.
#

ifeq ($(CONFIG_XPL_BUILD)$(CONFIG_TPL_BUILD),)
obj-$(CONFIG_TI_PWMSS)	+= ti-pwmss.o
obj-$(CONFIG_UNIPHIER_SYSTEM_BUS) += uniphier-system-bus.o
endif

obj-$(CONFIG_$(PHASE_)TI_SYSC)	+= ti-sysc.o
