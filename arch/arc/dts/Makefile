dtb-$(CONFIG_TARGET_ARCANGEL4) +=  arcangel4.dtb
dtb-$(CONFIG_TARGET_TB100) +=  abilis_tb100.dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
