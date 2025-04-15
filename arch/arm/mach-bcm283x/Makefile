# SPDX-License-Identifier: GPL-2.0
#
# (C) Copyright 2012 Stephen Warren

obj-$(CONFIG_BCM2835) += lowlevel_init.o
obj-y	+= init.o reset.o mbox.o msg.o phys2bus.o

ifeq ($(CONFIG_GENERATE_ACPI_TABLE),y)
obj-$(CONFIG_BCM2711) += bcm2711_acpi.o
endif