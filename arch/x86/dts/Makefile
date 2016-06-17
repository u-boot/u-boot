#
# SPDX-License-Identifier:	GPL-2.0+
#

dtb-y += bayleybay.dtb \
	chromebook_link.dtb \
	chromebox_panther.dtb \
	cougarcanyon2.dtb \
	crownbay.dtb \
	efi.dtb \
	galileo.dtb \
	minnowmax.dtb \
	qemu-x86_i440fx.dtb \
	qemu-x86_q35.dtb \
	broadwell_som-6896.dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
