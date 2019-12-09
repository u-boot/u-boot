# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2019 Google LLC

obj-$(CONFIG_SPL_BUILD) += cpu_spl.o
obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_SPL_BUILD) += systemagent.o
obj-y += cpu_common.o

ifndef CONFIG_TPL_BUILD
obj-y += cpu.o
obj-y += punit.o
endif

obj-y += hostbridge.o
obj-y += itss.o
obj-y += lpc.o
obj-y += pch.o
obj-y += pmc.o
obj-y += uart.o
