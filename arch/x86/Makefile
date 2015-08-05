#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CONFIG_EFI_APP),)
head-y := arch/x86/cpu/start.o
endif
ifeq ($(CONFIG_SPL_BUILD),y)
head-y += arch/x86/cpu/start16.o
head-y += arch/x86/cpu/resetvec.o
endif

libs-y += arch/x86/cpu/
libs-y += arch/x86/lib/
