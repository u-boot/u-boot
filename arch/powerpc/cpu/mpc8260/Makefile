#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o
obj-y	= traps.o serial_smc.o serial_scc.o cpu.o cpu_init.o speed.o \
	  interrupts.o ether_fcc.o i2c.o commproc.o \
	  bedbug_603e.o pci.o spi.o kgdb.o

obj-$(CONFIG_ETHER_ON_SCC) += ether_scc.o
