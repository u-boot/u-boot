#
# (C) Copyright 2016 Alexander Graf
#
#  SPDX-License-Identifier:     GPL-2.0+
#

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

CFLAGS_helloworld.o := $(CFLAGS_EFI)
CFLAGS_REMOVE_helloworld.o := $(CFLAGS_NON_EFI)

efiprogs-$(CONFIG_CMD_BOOTEFI_HELLO_COMPILE) += helloworld.efi
always := $(efiprogs-y)

obj-$(CONFIG_CMD_BOOTEFI_HELLO) += helloworld_efi.o
obj-y += efi_image_loader.o efi_boottime.o efi_runtime.o efi_console.o
obj-y += efi_memory.o
obj-$(CONFIG_LCD) += efi_gop.o
obj-$(CONFIG_PARTITIONS) += efi_disk.o
obj-$(CONFIG_NET) += efi_net.o
obj-$(CONFIG_GENERATE_SMBIOS_TABLE) += efi_smbios.o
