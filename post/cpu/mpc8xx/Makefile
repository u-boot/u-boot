#
# (C) Copyright 2002-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
include $(OBJTREE)/include/autoconf.mk

LIB	= libpostmpc8xx.o

AOBJS-$(CONFIG_HAS_POST)	+= cache_8xx.o
COBJS-$(CONFIG_HAS_POST)	+= cache.o ether.o spr.o uart.o usb.o watchdog.o

include $(TOPDIR)/post/rules.mk
