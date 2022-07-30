# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2021 Google LLC

obj-$(CONFIG_BOOTSTD) += bootdev.o bootstd_common.o bootflow.o bootmeth.o

ifdef CONFIG_OF_LIVE
obj-$(CONFIG_BOOTMETH_VBE_SIMPLE) += vbe_simple.o
endif
