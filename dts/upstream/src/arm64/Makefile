# SPDX-License-Identifier: GPL-2.0+

include $(srctree)/scripts/Makefile.dts

# Add any required device tree compiler flags here
DTC_FLAGS += -a 0x8

ifdef CONFIG_RCAR_64
	DTC_FLAGS += -R 4
endif
