# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

PLATFORM_CPPFLAGS += -fomit-frame-pointer
PF_CPPFLAGS_X86   := $(call cc-option, -fno-toplevel-reorder, \
		     $(call cc-option, -fno-unit-at-a-time))

PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_X86)
PLATFORM_CPPFLAGS += -fno-dwarf2-cfi-asm

ifdef CONFIG_XPL_BUILD
IS_32BIT := y
else
ifndef CONFIG_X86_64
IS_32BIT := y
endif
endif

EFI_IS_32BIT := $(IS_32BIT)
ifdef CONFIG_EFI_STUB_64BIT
EFI_IS_32BIT :=
endif

ifeq ($(IS_32BIT),y)
PLATFORM_CPPFLAGS += -march=i386 -m32
else
PLATFORM_CPPFLAGS += $(if $(CONFIG_XPL_BUILD),,-fpic) -fno-common -march=core2 -m64

ifndef CONFIG_X86_HARDFP
PLATFORM_CPPFLAGS += -mno-mmx -mno-sse
endif

endif # IS_32BIT

PLATFORM_RELFLAGS += -fdata-sections -ffunction-sections -fvisibility=hidden

KBUILD_LDFLAGS += -Bsymbolic -Bsymbolic-functions
KBUILD_LDFLAGS += -m $(if $(IS_32BIT),elf_i386,elf_x86_64)

# This is used in the top-level Makefile which does not include
# KBUILD_LDFLAGS
LDFLAGS_EFI_PAYLOAD := -Bsymbolic -Bsymbolic-functions -shared --no-undefined \
		       -s -zexecstack

OBJCOPYFLAGS_EFI := -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
	-j .rel -j .rela -j .reloc --strip-all

# Compiler flags to be added when building UEFI applications
CFLAGS_EFI := -fpic -fshort-wchar
# Compiler flags to be removed when building UEFI applications
CFLAGS_NON_EFI := -mregparm=3 -fstack-protector-strong

ifeq ($(IS_32BIT),y)
EFIPAYLOAD_BFDARCH = i386
else
CFLAGS_EFI += $(call cc-option, -mno-red-zone)
EFIPAYLOAD_BFDARCH = x86_64
endif

ifeq ($(EFI_IS_32BIT),y)
EFIARCH = ia32
EFIPAYLOAD_BFDTARGET = elf32-i386
else
EFIARCH = x86_64
EFIPAYLOAD_BFDTARGET = elf64-x86-64
endif

LDSCRIPT_EFI := $(srctree)/arch/x86/lib/elf_$(EFIARCH)_efi.lds
EFISTUB := crt0_$(EFIARCH)_efi.o reloc_$(EFIARCH)_efi.o
OBJCOPYFLAGS_EFI += --target=efi-app-$(EFIARCH)

CPPFLAGS_REMOVE_crt0-efi-$(EFIARCH).o += $(CFLAGS_NON_EFI)
CPPFLAGS_crt0-efi-$(EFIARCH).o += $(CFLAGS_EFI)

ifeq ($(CONFIG_EFI_APP),y)

PLATFORM_CPPFLAGS += $(CFLAGS_EFI)
LDFLAGS_FINAL += -znocombreloc -shared
LDSCRIPT := $(LDSCRIPT_EFI)

else

ifeq ($(IS_32BIT),y)
PLATFORM_CPPFLAGS += -mregparm=3
endif
KBUILD_LDFLAGS += --emit-relocs
LDFLAGS_FINAL += --gc-sections $(if $(CONFIG_XPL_BUILD),,-pie)

endif

ifdef CONFIG_X86_64
ifndef CONFIG_XPL_BUILD
PLATFORM_CPPFLAGS += -D__x86_64__
else
PLATFORM_CPPFLAGS += -D__I386__
endif
else
PLATFORM_CPPFLAGS += -D__I386__
endif

ifdef CONFIG_EFI_STUB

ifdef CONFIG_EFI_STUB_64BIT
EFI_LDS := elf_x86_64_efi.lds
EFI_CRT0 := crt0_x86_64_efi.o
EFI_RELOC := reloc_x86_64_efi.o
else
EFI_LDS := elf_ia32_efi.lds
EFI_CRT0 := crt0_ia32_efi.o
EFI_RELOC := reloc_ia32_efi.o
endif

else

ifdef CONFIG_X86_64
EFI_LDS := elf_x86_64_efi.lds
EFI_CRT0 := crt0_x86_64_efi.o
EFI_RELOC := reloc_x86_64_efi.o
else
EFI_LDS := elf_ia32_efi.lds
EFI_CRT0 := crt0_ia32_efi.o
EFI_RELOC := reloc_ia32_efi.o
endif

endif

ifdef CONFIG_X86_64
EFI_TARGET := --target=efi-app-x86_64
else
EFI_TARGET := --target=efi-app-ia32
endif
