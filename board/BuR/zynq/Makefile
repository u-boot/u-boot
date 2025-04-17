# SPDX-License-Identifier:	GPL-2.0+
#
# B&R Industrial Automation GmbH - http://www.br-automation.com

hw-platform-y :=$(shell echo $(CONFIG_DEFAULT_DEVICE_TREE) | sed -e 's/zynq-//')

obj-y := ../common/common.o
obj-y += ../common/br_resetc.o
obj-y += common/board.o
obj-y += $(hw-platform-y)/board.o

obj-$(CONFIG_SPL_BUILD) += $(hw-platform-y)/ps7_init_gpl.o

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_ps7_init_gpl.o := -Wstrict-prototypes
