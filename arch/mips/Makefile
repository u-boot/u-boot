#
# SPDX-License-Identifier:	GPL-2.0+
#

head-y := arch/mips/cpu/start.o

libs-y += arch/mips/cpu/
libs-y += arch/mips/lib/

machine-$(CONFIG_SOC_AU1X00) += au1x00

machdirs := $(patsubst %,arch/mips/mach-%/,$(machine-y))
libs-y += $(machdirs)

PLATFORM_CPPFLAGS += $(patsubst %,-I$(srctree)/%include,$(machdirs))
