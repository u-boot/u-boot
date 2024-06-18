# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2017 Marek Behún <kabel@kernel.org>

obj-y	:= turris_omnia.o ../turris_atsha_otp.o ../turris_common.o
obj-$(CONFIG_CMD_EEPROM_LAYOUT)	+= eeprom.o
obj-$(CONFIG_SPL_BUILD)		+= old_ddr3_training.o
