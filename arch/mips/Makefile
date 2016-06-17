#
# SPDX-License-Identifier:	GPL-2.0+
#

head-y := arch/mips/cpu/start.o

libs-y += arch/mips/cpu/
libs-y += arch/mips/lib/

machine-$(CONFIG_SOC_AU1X00) += au1x00
machine-$(CONFIG_MACH_PIC32) += pic32

machdirs := $(patsubst %,arch/mips/mach-%/,$(machine-y))
libs-y += $(machdirs)

PLATFORM_CPPFLAGS += $(patsubst %,-I$(srctree)/%include,$(machdirs))

# Optimize for MIPS architectures
arch-$(CONFIG_CPU_MIPS32_R1) += -march=mips32 -Wa,-mips32
arch-$(CONFIG_CPU_MIPS32_R2) += -march=mips32r2 -Wa,-mips32r2
arch-$(CONFIG_CPU_MIPS64_R1) += -march=mips64 -Wa,-mips64
arch-$(CONFIG_CPU_MIPS64_R2) += -march=mips64r2 -Wa,-mips64r2

# Allow extra optimization for specific CPUs/SoCs
tune-$(CONFIG_MIPS_TUNE_4KC) += -mtune=4kc
tune-$(CONFIG_MIPS_TUNE_14KC) += -mtune=14kc
tune-$(CONFIG_MIPS_TUNE_24KC) += -mtune=24kc

# Include default header files
cflags-y += -I$(srctree)/arch/mips/include/asm/mach-generic

PLATFORM_CPPFLAGS += $(arch-y) $(tune-y) $(cflags-y)
