#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += ashldi3.o ashrdi3.o lshrdi3.o \
				    lib1funcs.o uldivmod.o div0.o \
				    div64.o muldi3.o

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

obj-$(CONFIG_CPU_V7M) += cmd_boot.o
obj-$(CONFIG_OF_LIBFDT) += bootm-fdt.o
obj-$(CONFIG_CMD_BOOTI) += bootm.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-$(CONFIG_CMD_BOOTZ) += bootm.o zimage.o
obj-$(CONFIG_SYS_L2_PL310) += cache-pl310.o
obj-$(CONFIG_USE_ARCH_MEMSET) += memset.o
obj-$(CONFIG_USE_ARCH_MEMCPY) += memcpy.o
else
obj-$(CONFIG_SPL_FRAMEWORK) += spl.o
obj-$(CONFIG_SPL_FRAMEWORK) += zimage.o
endif
obj-$(CONFIG_SEMIHOSTING) += semihosting.o

obj-y	+= sections.o
obj-y	+= stack.o
ifdef CONFIG_CPU_V7M
obj-y	+= interrupts_m.o
else ifdef CONFIG_ARM64
obj-y	+= ccn504.o
obj-y	+= gic_64.o
obj-y	+= interrupts_64.o
else
obj-y	+= interrupts.o
endif
ifndef CONFIG_SYSRESET
obj-y	+= reset.o
endif

obj-y	+= cache.o
ifndef CONFIG_ARM64
obj-y	+= cache-cp15.o
endif

obj-y	+= psci-dt.o

obj-$(CONFIG_DEBUG_LL)	+= debug.o

# For EABI conformant tool chains, provide eabi_compat()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
extra-y	+= eabi_compat.o
endif

asflags-y += -DCONFIG_ARM_ASM_UNIFIED

# some files can only build in ARM or THUMB2, not THUMB1

ifdef CONFIG_SYS_THUMB_BUILD
asflags-$(CONFIG_HAS_THUMB2) += -DCONFIG_THUMB2_KERNEL
ifndef CONFIG_HAS_THUMB2

# for C files, just apend -marm, which will override previous -mthumb*

CFLAGS_cache.o := -marm
CFLAGS_cache-cp15.o := -marm

# For .S, drop -mthumb* and other thumb-related options.
# CFLAGS_REMOVE_* would not have an effet, so AFLAGS_REMOVE_*
# was implemented and is used here.
# Also, define ${target}_NO_THUMB_BUILD for these two targets
# so that the code knows it should not use Thumb.

AFLAGS_REMOVE_memset.o := -mthumb -mthumb-interwork
AFLAGS_REMOVE_memcpy.o := -mthumb -mthumb-interwork
AFLAGS_memset.o := -DMEMSET_NO_THUMB_BUILD
AFLAGS_memcpy.o := -DMEMCPY_NO_THUMB_BUILD
endif
endif
