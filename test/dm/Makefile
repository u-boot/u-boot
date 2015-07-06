#
# Copyright (c) 2013 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_CMD_DM) += cmd_dm.o
obj-$(CONFIG_UT_DM) += bus.o
obj-$(CONFIG_UT_DM) += test-driver.o
obj-$(CONFIG_UT_DM) += test-fdt.o
obj-$(CONFIG_UT_DM) += test-main.o
obj-$(CONFIG_UT_DM) += test-uclass.o

# Tests for particular subsystems - when enabling driver model for a new
# subsystem you must add sandbox tests here.
obj-$(CONFIG_UT_DM) += core.o
ifneq ($(CONFIG_SANDBOX),)
obj-$(CONFIG_CLK) += clk.o
obj-$(CONFIG_DM_ETH) += eth.o
obj-$(CONFIG_DM_GPIO) += gpio.o
obj-$(CONFIG_DM_I2C) += i2c.o
obj-$(CONFIG_LED) += led.o
obj-$(CONFIG_DM_MMC) += mmc.o
obj-$(CONFIG_DM_PCI) += pci.o
obj-$(CONFIG_RAM) += ram.o
obj-y += regmap.o
obj-$(CONFIG_RESET) += reset.o
obj-$(CONFIG_DM_RTC) += rtc.o
obj-$(CONFIG_DM_SPI_FLASH) += sf.o
obj-$(CONFIG_DM_SPI) += spi.o
obj-y += syscon.o
obj-$(CONFIG_DM_USB) += usb.o
obj-$(CONFIG_DM_PMIC) += pmic.o
obj-$(CONFIG_DM_REGULATOR) += regulator.o
endif
