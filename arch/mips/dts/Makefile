#
# SPDX-License-Identifier:	GPL-2.0+
#

dtb-$(CONFIG_TARGET_PIC32MZDASK) += pic32mzda_sk.dtb

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
