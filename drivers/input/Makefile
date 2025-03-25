# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_$(PHASE_)CROS_EC_KEYB) += cros_ec_keyb.o
obj-$(CONFIG_$(PHASE_)OF_CONTROL) += key_matrix.o
obj-$(CONFIG_$(PHASE_)DM_KEYBOARD) += input.o keyboard-uclass.o
obj-$(CONFIG_BUTTON_KEYBOARD) += button_kbd.o
obj-$(CONFIG_CPCAP_POWER_BUTTON) += cpcap_pwrbutton.o

ifndef CONFIG_XPL_BUILD

obj-$(CONFIG_APPLE_SPI_KEYB) += apple_spi_kbd.o
obj-$(CONFIG_I8042_KEYB) += i8042.o
obj-$(CONFIG_TEGRA_KEYBOARD) += input.o tegra-kbc.o
obj-$(CONFIG_TWL4030_INPUT) += twl4030.o
endif
