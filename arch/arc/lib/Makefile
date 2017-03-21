#
# Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o
head-y := start.o
obj-y += cache.o
obj-y += cpu.o
obj-y += interrupts.o
obj-y += relocate.o
obj-y += strchr-700.o
obj-y += strcmp.o
obj-y += strcpy-700.o
obj-y += strlen.o
obj-y += memcmp.o
obj-y += memcpy-700.o
obj-y += memset.o
obj-y += reset.o
obj-y += ints_low.o
obj-y += init_helpers.o

obj-$(CONFIG_CMD_BOOTM) += bootm.o

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += _millicodethunk.o libgcc2.o
