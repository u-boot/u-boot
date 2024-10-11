# Build a combined spl + u-boot image
ifdef CONFIG_SPL
ifndef CONFIG_XPL_BUILD
ALL-y += u-boot-sunxi-with-spl.bin
endif
endif
