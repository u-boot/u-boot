#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_EFI_STUB) += car.o
obj-$(CONFIG_EFI_STUB) += efi.o

obj-$(CONFIG_EFI_APP) += crt0-efi-ia32.o reloc_ia32.o

ifneq ($(CONFIG_EFI_STUB),)

CFLAGS_REMOVE_reloc_ia32.o += -mregparm=3
CFLAGS_reloc_ia32.o += -fpic -fshort-wchar

# When building for 64-bit we must remove the i386-specific flags
CFLAGS_REMOVE_reloc_x86_64.o += -mregparm=3 -march=i386 -m32
CFLAGS_reloc_x86_64.o += -fpic -fshort-wchar

AFLAGS_REMOVE_crt0-efi-x86_64.o += -mregparm=3 -march=i386 -m32
AFLAGS_crt0-efi-x86_64.o += -fpic -fshort-wchar

extra-$(CONFIG_EFI_STUB_32BIT) += crt0-efi-ia32.o reloc_ia32.o
extra-$(CONFIG_EFI_STUB_64BIT) += crt0-efi-x86_64.o reloc_x86_64.o
endif
