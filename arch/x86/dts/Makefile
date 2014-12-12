dtb-y += link.dtb \
	chromebook_link.dtb \
	alex.dtb \
	crownbay.dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
