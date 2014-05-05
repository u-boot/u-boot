# Build a combined spl + u-boot image
ifdef CONFIG_SPL
ifndef CONFIG_SPL_BUILD
ifndef CONFIG_SPL_FEL
ALL-y += u-boot-sunxi-with-spl.bin
endif
endif
endif
