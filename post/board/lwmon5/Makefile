#
# (C) Copyright 2008 Dmitry Rakhchev, EmCraft Systems, rda@emcraft.com
#
# Developed for DENX Software Engineering GmbH
#
# SPDX-License-Identifier:	GPL-2.0+
include $(OBJTREE)/include/autoconf.mk

LIB	= libpostlwmon5.o

COBJS-$(CONFIG_HAS_POST)	+= sysmon.o watchdog.o dspic.o fpga.o dsp.o gdc.o

include $(TOPDIR)/post/rules.mk
