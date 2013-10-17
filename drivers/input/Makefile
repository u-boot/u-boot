#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_I8042_KBD) += i8042.o
obj-$(CONFIG_TEGRA_KEYBOARD) += tegra-kbc.o
obj-$(CONFIG_CROS_EC_KEYB) += cros_ec_keyb.o
ifdef CONFIG_PS2KBD
obj-y += keyboard.o pc_keyb.o
obj-$(CONFIG_PS2MULT) += ps2mult.o ps2ser.o
endif
obj-y += input.o
obj-$(CONFIG_OF_CONTROL) += key_matrix.o
