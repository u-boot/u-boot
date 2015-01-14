#
# Copyright (c) 2013 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_CMD_DM) += cmd_dm.o
obj-$(CONFIG_DM_TEST) += bus.o
obj-$(CONFIG_DM_TEST) += test-driver.o
obj-$(CONFIG_DM_TEST) += test-fdt.o
obj-$(CONFIG_DM_TEST) += test-main.o
obj-$(CONFIG_DM_TEST) += test-uclass.o
obj-$(CONFIG_DM_TEST) += ut.o

# Tests for particular subsystems - when enabling driver model for a new
# subsystem you must add sandbox tests here.
obj-$(CONFIG_DM_TEST) += core.o
obj-$(CONFIG_DM_TEST) += ut.o
ifneq ($(CONFIG_SANDBOX),)
obj-$(CONFIG_DM_GPIO) += gpio.o
obj-$(CONFIG_DM_SPI) += spi.o
obj-$(CONFIG_DM_SPI_FLASH) += sf.o
obj-$(CONFIG_DM_I2C) += i2c.o
endif
