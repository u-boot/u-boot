#
# Copyright (c) 2014 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_HAVE_FSP
obj-y += fsp_configs.o ivybridge.o
else
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += cpu.o
obj-y += early_me.o
obj-y += lpc.o
obj-y += model_206ax.o
obj-y += northbridge.o
obj-y += sata.o
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += sdram.o
endif
obj-y += bd82x6x.o
