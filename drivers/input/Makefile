#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_DM_KEYBOARD) += keyboard-uclass.o

obj-$(CONFIG_I8042_KEYB) += i8042.o
obj-$(CONFIG_TEGRA_KEYBOARD) += tegra-kbc.o
obj-$(CONFIG_TWL4030_INPUT) += twl4030.o
obj-$(CONFIG_TWL6030_INPUT) += twl6030.o
obj-$(CONFIG_CROS_EC_KEYB) += cros_ec_keyb.o
ifdef CONFIG_PS2KBD
obj-y += keyboard.o pc_keyb.o
obj-$(CONFIG_PS2MULT) += ps2mult.o ps2ser.o
endif
obj-y += input.o
obj-$(CONFIG_$(SPL_)OF_CONTROL) += key_matrix.o
