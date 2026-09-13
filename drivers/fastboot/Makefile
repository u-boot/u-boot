# SPDX-License-Identifier: GPL-2.0+

obj-y += fb_common.o
obj-y += fb_getvar.o
obj-y += fb_command.o
obj-$(CONFIG_$(PHASE_)USB_FUNCTION_FASTBOOT) += fb_usb.o
obj-$(CONFIG_$(PHASE_)FASTBOOT_FLASH_BLOCK) += fb_block.o
# MMC reuses block implementation
obj-$(CONFIG_$(PHASE_)FASTBOOT_FLASH_MMC) += fb_block.o fb_mmc.o
obj-$(CONFIG_$(PHASE_)FASTBOOT_FLASH_NAND) += fb_nand.o
obj-$(CONFIG_$(PHASE_)FASTBOOT_FLASH_SPI) += fb_spi_flash.o
