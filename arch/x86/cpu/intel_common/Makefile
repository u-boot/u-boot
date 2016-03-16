#
# Copyright (c) 2016 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_HAVE_MRC) += car.o
obj-y += cpu.o
obj-y += lpc.o
obj-$(CONFIG_HAVE_MRC) += me_status.o
ifndef CONFIG_TARGET_EFI
obj-y += microcode.o
endif
obj-y += pch.o
obj-$(CONFIG_HAVE_MRC) += report_platform.o
obj-$(CONFIG_HAVE_MRC) += mrc.o
