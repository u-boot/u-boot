#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += bios.o
obj-y += bios_asm.o
obj-y += bios_interrupts.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-y	+= cmd_boot.o
obj-y	+= coreboot_table.o
obj-$(CONFIG_EFI) += efi/
obj-y	+= e820.o
obj-y	+= gcc.o
obj-y	+= init_helpers.o
obj-y	+= interrupts.o
obj-y	+= lpc-uclass.o
obj-y	+= mpspec.o
obj-$(CONFIG_ENABLE_MRC_CACHE) += mrccache.o
obj-y += cmd_mtrr.o
obj-y	+= northbridge-uclass.o
obj-$(CONFIG_I8259_PIC) += i8259.o
obj-$(CONFIG_I8254_TIMER) += i8254.o
obj-y	+= pinctrl_ich6.o
obj-y	+= pirq_routing.o
obj-y	+= relocate.o
obj-y += physmem.o
obj-$(CONFIG_X86_RAMTEST) += ramtest.o
obj-y += sfi.o
obj-$(CONFIG_GENERATE_SMBIOS_TABLE) += smbios.o
obj-y	+= string.o
ifndef CONFIG_QEMU_ACPI_TABLE
obj-$(CONFIG_GENERATE_ACPI_TABLE) += acpi_table.o
endif
obj-y	+= tables.o
obj-$(CONFIG_CMD_ZBOOT)	+= zimage.o
obj-$(CONFIG_HAVE_FSP) += fsp/

extra-$(CONFIG_USE_PRIVATE_LIBGCC) += lib.a

NORMAL_LIBGCC = $(shell $(CC) $(PLATFORM_CPPFLAGS) -print-libgcc-file-name)
OBJCOPYFLAGS := --prefix-symbols=__normal_
$(obj)/lib.a: $(NORMAL_LIBGCC) FORCE
	$(call if_changed,objcopy)
