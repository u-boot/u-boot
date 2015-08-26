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
obj-$(CONFIG_HAVE_FSP) += cmd_hob.o
obj-$(CONFIG_EFI) += efi/
obj-y	+= gcc.o
obj-y	+= init_helpers.o
obj-y	+= interrupts.o
obj-y	+= lpc-uclass.o
obj-y	+= mpspec.o
obj-y += cmd_mtrr.o
obj-$(CONFIG_SYS_PCAT_INTERRUPTS) += pcat_interrupts.o
obj-$(CONFIG_SYS_PCAT_TIMER) += pcat_timer.o
ifndef CONFIG_DM_PCI
obj-$(CONFIG_PCI) += pci_type1.o
endif
obj-y	+= pch-uclass.o
obj-y	+= pirq_routing.o
obj-y	+= relocate.o
obj-y += physmem.o
obj-$(CONFIG_X86_RAMTEST) += ramtest.o
obj-y += sfi.o
obj-y	+= string.o
obj-$(CONFIG_GENERATE_ACPI_TABLE) += acpi_table.o
obj-y	+= tables.o
obj-$(CONFIG_SYS_X86_TSC_TIMER)	+= tsc_timer.o
obj-$(CONFIG_CMD_ZBOOT)	+= zimage.o
obj-$(CONFIG_HAVE_FSP) += fsp/

extra-$(CONFIG_USE_PRIVATE_LIBGCC) += lib.a

NORMAL_LIBGCC = $(shell $(CC) $(PLATFORM_CPPFLAGS) -print-libgcc-file-name)
OBJCOPYFLAGS := --prefix-symbols=__normal_
$(obj)/lib.a: $(NORMAL_LIBGCC) FORCE
	$(call if_changed,objcopy)
