#
# Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_EFI_STUB
obj-y += car.o dram.o
endif
obj-y += qemu.o
obj-$(CONFIG_GENERATE_ACPI_TABLE) += acpi.o dsdt.o
obj-$(CONFIG_PCI) += pci.o
