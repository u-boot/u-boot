#
# SPDX-License-Identifier:	GPL-2.0+
#

head-y := arch/x86/cpu/start.o
ifeq ($(CONFIG_SPL_BUILD),y)
head-y += arch/x86/cpu/start16.o
head-y += arch/x86/cpu/resetvec.o
endif

libs-y += arch/x86/cpu/
libs-y += arch/x86/lib/
