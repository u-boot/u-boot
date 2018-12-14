# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

CFLAGS_cpu.o += -finline-limit=64000

obj-y += cpu.o dram.o reset.o lowlevel_init.o
obj-$(CONFIG_SOC_LUTON) += lowlevel_init_luton.o
