#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

COBJS-libfdt += fdt.o fdt_ro.o fdt_rw.o fdt_strerror.o fdt_sw.o fdt_wip.o \
	fdt_empty_tree.o fdt_addresses.o

obj-$(CONFIG_OF_LIBFDT) += $(COBJS-libfdt)
obj-$(CONFIG_FIT) += $(COBJS-libfdt)
