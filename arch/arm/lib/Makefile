#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += _ashldi3.o _ashrdi3.o _divsi3.o \
			_lshrdi3.o _modsi3.o _udivsi3.o _umodsi3.o div0.o

ifdef CONFIG_CPU_V7M
obj-y	+= vectors_m.o crt0.o
else ifdef CONFIG_ARM64
obj-y	+= crt0_64.o
else
obj-y	+= vectors.o crt0.o
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

obj-$(CONFIG_CPU_V7M) += cmd_boot.o
obj-$(CONFIG_OF_LIBFDT) += bootm-fdt.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-$(CONFIG_SYS_L2_PL310) += cache-pl310.o
obj-$(CONFIG_USE_ARCH_MEMSET) += memset.o
obj-$(CONFIG_USE_ARCH_MEMCPY) += memcpy.o
else
obj-$(CONFIG_SPL_FRAMEWORK) += spl.o
endif
obj-$(CONFIG_SEMIHOSTING) += semihosting.o

obj-y	+= sections.o
obj-y	+= stack.o
ifdef CONFIG_CPU_V7M
obj-y	+= interrupts_m.o
else ifdef CONFIG_ARM64
obj-y	+= gic_64.o
obj-y	+= interrupts_64.o
else
obj-y	+= interrupts.o
endif
obj-y	+= reset.o

obj-y	+= cache.o
ifndef CONFIG_ARM64
obj-y	+= cache-cp15.o
endif

obj-$(CONFIG_DEBUG_LL)	+= debug.o

# For EABI conformant tool chains, provide eabi_compat()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
extra-y	+= eabi_compat.o
endif
