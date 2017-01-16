#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CONFIG_EFI_APP),)
head-y := arch/x86/cpu/start.o
endif
head-$(CONFIG_$(SPL_)X86_16BIT_INIT) += arch/x86/cpu/start16.o
head-$(CONFIG_$(SPL_)X86_16BIT_INIT) += arch/x86/cpu/resetvec.o

libs-y += arch/x86/cpu/
libs-y += arch/x86/lib/
