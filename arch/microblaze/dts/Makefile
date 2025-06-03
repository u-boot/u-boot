# SPDX-License-Identifier: GPL-2.0+

dtb-y += $(shell echo $(CONFIG_DEFAULT_DEVICE_TREE)).dtb

include $(srctree)/scripts/Makefile.dts

# Add any required device tree compiler flags here
DTC_FLAGS += -R 4
