#
# (C) Copyright 2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
include $(TOPDIR)/config.mk

LIB	= libpost$(ARCH)fpu.o

COBJS-$(CONFIG_HAS_POST)	+= 20001122-1.o
COBJS-$(CONFIG_HAS_POST)	+= 20010114-2.o
COBJS-$(CONFIG_HAS_POST)	+= 20010226-1.o
COBJS-$(CONFIG_HAS_POST)	+= 980619-1.o
COBJS-$(CONFIG_HAS_POST)	+= acc1.o
COBJS-$(CONFIG_HAS_POST)	+= compare-fp-1.o
COBJS-$(CONFIG_HAS_POST)	+= fpu.o
COBJS-$(CONFIG_HAS_POST)	+= mul-subnormal-single-1.o

COBJS-$(CONFIG_HAS_POST)	+= darwin-ldouble.o

include $(TOPDIR)/post/rules.mk

CFLAGS := $(shell echo $(CFLAGS) | sed s/-msoft-float//)
CFLAGS += -mhard-float -fkeep-inline-functions

$(obj)%.o:	%.c
	$(CC)  $(ALL_CFLAGS) -o $@.fp $< -c
	$(OBJCOPY) -R .gnu.attributes $@.fp $@
	rm -f $@.fp
