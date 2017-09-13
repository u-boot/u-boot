#
# (C) Copyright 2016 Alexander Graf
#
#  SPDX-License-Identifier:     GPL-2.0+
#

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

CFLAGS_helloworld.o := $(CFLAGS_EFI)
CFLAGS_REMOVE_helloworld.o := $(CFLAGS_NON_EFI)

ifneq ($(CONFIG_CMD_BOOTEFI_HELLO_COMPILE),)
always += helloworld.efi
endif

obj-$(CONFIG_CMD_BOOTEFI_HELLO) += helloworld_efi.o
obj-y += efi_image_loader.o efi_boottime.o efi_runtime.o efi_console.o
obj-y += efi_memory.o efi_device_path_to_text.o efi_device_path.o
obj-y += efi_file.o efi_variable.o efi_bootmgr.o
obj-$(CONFIG_LCD) += efi_gop.o
obj-$(CONFIG_DM_VIDEO) += efi_gop.o
obj-$(CONFIG_PARTITIONS) += efi_disk.o
obj-$(CONFIG_NET) += efi_net.o
obj-$(CONFIG_GENERATE_SMBIOS_TABLE) += efi_smbios.o
