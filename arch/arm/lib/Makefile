#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# Build private libgcc only when asked for
ifdef USE_PRIVATE_LIBGCC
lib-y	+= _ashldi3.o
lib-y	+= _ashrdi3.o
lib-y	+= _divsi3.o
lib-y	+= _lshrdi3.o
lib-y	+= _modsi3.o
lib-y	+= _udivsi3.o
lib-y	+= _umodsi3.o
lib-y	+= div0.o
endif

ifdef CONFIG_ARM64
obj-y	+= crt0_64.o
else
obj-y	+= crt0.o
endif

ifndef CONFIG_SPL_BUILD
ifdef CONFIG_ARM64
obj-y	+= relocate_64.o
else
obj-y	+= relocate.o
endif
ifndef CONFIG_SYS_GENERIC_BOARD
obj-y	+= board.o
endif

obj-$(CONFIG_OF_LIBFDT) += bootm-fdt.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-$(CONFIG_SYS_L2_PL310) += cache-pl310.o
obj-$(CONFIG_USE_ARCH_MEMSET) += memset.o
obj-$(CONFIG_USE_ARCH_MEMCPY) += memcpy.o
else
obj-$(CONFIG_SPL_FRAMEWORK) += spl.o
endif

obj-y	+= sections.o
ifdef CONFIG_ARM64
obj-y	+= interrupts_64.o
else
obj-y	+= interrupts.o
endif
obj-y	+= reset.o

obj-y	+= cache.o
ifndef CONFIG_ARM64
obj-y	+= cache-cp15.o
endif

# For EABI conformant tool chains, provide eabi_compat()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
extra-y	+= eabi_compat.o
endif
