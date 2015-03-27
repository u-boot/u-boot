#
# (C) Copyright 2007-2009 DENX Software Engineering
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o
obj-y	:= cpu.o
obj-y	+= traps.o
obj-y += cpu_init.o
obj-y += fixed_sdram.o
obj-y += i2c.o
obj-y += interrupts.o
obj-y += iopin.o
obj-y += serial.o
obj-y += speed.o
obj-$(CONFIG_FSL_DIU_FB) += diu.o
obj-$(CONFIG_CMD_IDE) += ide.o
obj-$(CONFIG_PCI) += pci.o
