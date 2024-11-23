# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2019 Siemens AG
#

obj-y += board.o
obj-y += ../common/eeprom.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o
obj-$(CONFIG_SPL_CMT) += spl_memory_test.o
else
obj-y += ../common/factoryset.o
obj-$(CONFIG_DDR_SI_TEST) += ../common/ddr_si_test.o
endif
