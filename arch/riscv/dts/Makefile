# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_AX25_AE350) += ae350_32.dtb ae350_64.dtb
dtb-$(CONFIG_TARGET_SIFIVE_FU540) += hifive-unleashed-a00.dtb
dtb-$(CONFIG_TARGET_SIPEED_MAIX) += k210-maix-bit.dtb
dtb-$(CONFIG_TARGET_MICROCHIP_ICICLE) += microchip-mpfs-icicle-kit.dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
