#
# (C) Copyright 2016 Alexander Graf
#
#  SPDX-License-Identifier:     GPL-2.0+
#

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

obj-y += efi_image_loader.o efi_boottime.o efi_runtime.o efi_console.o
obj-y += efi_memory.o
obj-$(CONFIG_LCD) += efi_gop.o
obj-$(CONFIG_PARTITIONS) += efi_disk.o
obj-$(CONFIG_NET) += efi_net.o
