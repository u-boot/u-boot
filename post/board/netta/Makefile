#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
include $(OBJTREE)/include/autoconf.mk

LIB	= libpostnetta.o

COBJS-$(CONFIG_HAS_POST)	+= codec.o dsp.o

include $(TOPDIR)/post/rules.mk
