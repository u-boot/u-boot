#
# Copyright (C) 2012 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_SOFT_I2C_MULTI_BUS) += multi_i2c.o
obj-$(CONFIG_THOR_FUNCTION) += thor.o
obj-$(CONFIG_CMD_USB_MASS_STORAGE) += ums.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_BOARD_COMMON)	+= board.o
endif
