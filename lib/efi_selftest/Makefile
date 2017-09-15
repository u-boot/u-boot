:
# (C) Copyright 2017, Heinrich Schuchardt <xypron.glpk@gmx.de>
#
#  SPDX-License-Identifier:     GPL-2.0+
#

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

CFLAGS_efi_selftest.o := $(CFLAGS_EFI)
CFLAGS_REMOVE_efi_selftest.o := $(CFLAGS_NON_EFI)
CFLAGS_efi_selftest_console.o := $(CFLAGS_EFI)
CFLAGS_REMOVE_efi_selftest_console.o := $(CFLAGS_NON_EFI)
CFLAGS_efi_selftest_events.o := $(CFLAGS_EFI)
CFLAGS_REMOVE_efi_selftest_events.o := $(CFLAGS_NON_EFI)
CFLAGS_efi_selftest_tpl.o := $(CFLAGS_EFI)
CFLAGS_REMOVE_efi_selftest_tpl.o := $(CFLAGS_NON_EFI)

obj-$(CONFIG_CMD_BOOTEFI_SELFTEST) += \
efi_selftest.o \
efi_selftest_console.o \
efi_selftest_events.o \
efi_selftest_tpl.o
