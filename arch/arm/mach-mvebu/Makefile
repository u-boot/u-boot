#
# Copyright (C) 2014-2015 Stefan Roese <sr@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_KIRKWOOD

obj-y	= dram.o
obj-y	+= gpio.o
obj-y	+= timer.o

else

obj-y	= cpu.o
obj-y	+= dram.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMADA_38X) += ../../../drivers/ddr/marvell/a38x/xor.o
obj-$(CONFIG_ARMADA_XP) += ../../../drivers/ddr/marvell/axp/xor.o
endif
obj-y	+= gpio.o
obj-y	+= mbus.o
obj-y	+= timer.o
obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_SPL_BUILD) += lowlevel_spl.o

obj-$(CONFIG_ARMADA_38X) += serdes/a38x/
obj-$(CONFIG_ARMADA_XP) += serdes/axp/

endif
