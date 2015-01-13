#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o
extra-y += traps.o
obj-y  += cache.o
obj-y  += io.o
obj-y  += firmware_sc_task_bestcomm.impl.o
obj-y += i2c.o
obj-y += cpu.o
obj-y += cpu_init.o
obj-y += ide.o
obj-y += interrupts.o
obj-y += loadtask.o
obj-y += pci_mpc5200.o
obj-y += serial.o
obj-y += speed.o
obj-$(CONFIG_CMD_USB) += usb_ohci.o
obj-$(CONFIG_CMD_USB) += usb.o

ifdef CONFIG_SPL_BUILD
obj-y += spl_boot.o
endif
