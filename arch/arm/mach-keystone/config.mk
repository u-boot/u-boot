# Copyright 2015 Texas Instruments Incorporated, <www.ti.com>
#
# Lokesh Vutla <lokeshvutla@ti.com>
#
# SPDX-License-Identifier:     GPL-2.0+
#

ifndef CONFIG_SPL_BUILD
ALL-y += MLO
endif

MKIMAGEFLAGS_u-boot-spl.gph = -A $(ARCH) -T gpimage -C none \
	-a $(CONFIG_SPL_TEXT_BASE) -e $(CONFIG_SPL_TEXT_BASE) -n SPL
spl/u-boot-spl.gph: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

OBJCOPYFLAGS_u-boot-spi.gph = -I binary -O binary --pad-to=$(CONFIG_SPL_PAD_TO) \
			  --gap-fill=0
u-boot-spi.gph: spl/u-boot-spl.gph u-boot-dtb.img FORCE
	$(call if_changed,pad_cat)

ifndef CONFIG_SPL_BUILD
MKIMAGEFLAGS_MLO = -A $(ARCH) -T gpimage -C none \
	-a $(CONFIG_SYS_TEXT_BASE) -e $(CONFIG_SYS_TEXT_BASE) -n U-Boot
MLO: u-boot-dtb.bin FORCE
	$(call if_changed,mkimage)
	@dd if=/dev/zero bs=8 count=1 2>/dev/null >> $@
endif
