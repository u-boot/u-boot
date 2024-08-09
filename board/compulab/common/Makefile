# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2011 - 2013 CompuLab, Ltd. <www.compulab.co.il>
#
# Author: Igor Grinberg <grinberg@compulab.co.il>

CL_EEPROM=y

ifdef CONFIG_TARGET_TRIMSLICE
CL_EEPROM=
endif

obj-y				+= common.o
obj-$(CL_EEPROM)		+= eeprom.o
obj-$(CONFIG_SMC911X)		+= omap3_smc911x.o
