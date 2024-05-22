# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2024 Jiaxun yang <jiaxun.yang@flygoat.com>
#

obj-$(CONFIG_$(PHASE_)LIB_BOOTM) += bootm.o
obj-$(CONFIG_$(PHASE_)LIB_BOOTI) += image.o
obj-$(CONFIG_CMD_GO) += boot.o
obj-y	+= cache.o
obj-y	+= interrupts.o
ifeq ($(CONFIG_$(SPL_)SYSRESET),)
obj-y	+= reset.o
endif
obj-y	+= setjmp.o

# For building EFI apps
CFLAGS_NON_EFI := -fstack-protector-strong
CFLAGS_$(EFI_CRT0) := $(CFLAGS_EFI)
CFLAGS_REMOVE_$(EFI_CRT0) := $(CFLAGS_NON_EFI)

CFLAGS_$(EFI_RELOC) := $(CFLAGS_EFI)
CFLAGS_REMOVE_$(EFI_RELOC) := $(CFLAGS_NON_EFI)

extra-$(CONFIG_CMD_BOOTEFI_HELLO_COMPILE) += $(EFI_CRT0) $(EFI_RELOC)
extra-$(CONFIG_CMD_BOOTEFI_SELFTEST) += $(EFI_CRT0) $(EFI_RELOC)
extra-$(CONFIG_EFI) += $(EFI_CRT0) $(EFI_RELOC)
