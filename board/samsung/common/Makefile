#
# Copyright (C) 2012 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_SOFT_I2C_MULTI_BUS) += multi_i2c.o
obj-$(CONFIG_USB_GADGET_DOWNLOAD) += gadget.o
obj-$(CONFIG_MISC_COMMON) += misc.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_BOARD_COMMON)	+= board.o
ifdef CONFIG_EXYNOS5_DT
obj-y += exynos5-dt.o
obj-$(CONFIG_BOARD_TYPES) += exynos5-dt-types.o
endif
endif
