#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
include $(TOPDIR)/config.mk

LIB	= libpost$(ARCH).o

AOBJS-$(CONFIG_HAS_POST)	+= asm.o
COBJS-$(CONFIG_HAS_POST)	+= cpu.o cmp.o cmpi.o two.o twox.o three.o threex.o
COBJS-$(CONFIG_HAS_POST)   += threei.o andi.o srawi.o rlwnm.o rlwinm.o rlwimi.o
COBJS-$(CONFIG_HAS_POST)	+= store.o load.o cr.o b.o multi.o string.o complex.o

include $(TOPDIR)/post/rules.mk
