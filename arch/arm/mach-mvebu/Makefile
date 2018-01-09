#
# Copyright (C) 2014-2016 Stefan Roese <sr@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_ARM64

obj-$(CONFIG_ARMADA_3700) += armada3700/
obj-$(CONFIG_ARMADA_8K) += armada8k/
obj-y += arm64-common.o
obj-y += sata.o

else # CONFIG_ARM64

ifdef CONFIG_KIRKWOOD

obj-y	= dram.o
obj-y	+= gpio.o
obj-y	+= timer.o

else # CONFIG_KIRKWOOD

obj-y	= cpu.o
obj-y	+= dram.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMADA_375) += ../../../drivers/ddr/marvell/axp/xor.o
obj-$(CONFIG_ARMADA_38X) += ../../../drivers/ddr/marvell/a38x/xor.o
obj-$(CONFIG_ARMADA_XP) += ../../../drivers/ddr/marvell/axp/xor.o
obj-$(CONFIG_MVEBU_EFUSE) += efuse.o
endif # CONFIG_SPL_BUILD
obj-y	+= gpio.o
obj-y	+= mbus.o
obj-y	+= timer.o
obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_SPL_BUILD) += lowlevel_spl.o

obj-$(CONFIG_ARMADA_38X) += serdes/a38x/
obj-$(CONFIG_ARMADA_XP) += serdes/axp/

endif # CONFIG_KIRKWOOD
endif # CONFIG_ARM64
