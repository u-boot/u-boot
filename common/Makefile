#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# core
ifndef CONFIG_SPL_BUILD
obj-y += init/
obj-y += main.o
obj-y += exports.o
obj-y += hash.o
obj-$(CONFIG_HUSH_PARSER) += cli_hush.o
obj-$(CONFIG_AUTOBOOT) += autoboot.o

# This option is not just y/n - it can have a numeric value
ifdef CONFIG_BOOT_RETRY_TIME
obj-y += bootretry.o
endif

# boards
obj-y += board_f.o
obj-y += board_r.o
obj-$(CONFIG_DISPLAY_BOARDINFO) += board_info.o
obj-$(CONFIG_DISPLAY_BOARDINFO_LATE) += board_info.o

obj-$(CONFIG_CMD_BOOTM) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTZ) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTI) += bootm.o bootm_os.o

# environment
obj-y += env_attr.o
obj-y += env_callback.o
obj-y += env_flags.o
obj-$(CONFIG_ENV_IS_IN_DATAFLASH) += env_dataflash.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += env_eeprom.o
extra-$(CONFIG_ENV_IS_EMBEDDED) += env_embedded.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += env_embedded.o
extra-$(CONFIG_ENV_IS_IN_FLASH) += env_embedded.o
obj-$(CONFIG_ENV_IS_IN_NVRAM) += env_embedded.o
obj-$(CONFIG_ENV_IS_IN_FLASH) += env_flash.o
obj-$(CONFIG_ENV_IS_IN_MMC) += env_mmc.o
obj-$(CONFIG_ENV_IS_IN_FAT) += env_fat.o
obj-$(CONFIG_ENV_IS_IN_EXT4) += env_ext4.o
obj-$(CONFIG_ENV_IS_IN_NAND) += env_nand.o
obj-$(CONFIG_ENV_IS_IN_NVRAM) += env_nvram.o
obj-$(CONFIG_ENV_IS_IN_ONENAND) += env_onenand.o
obj-$(CONFIG_ENV_IS_IN_SATA) += env_sata.o
obj-$(CONFIG_ENV_IS_IN_SPI_FLASH) += env_sf.o
obj-$(CONFIG_ENV_IS_IN_REMOTE) += env_remote.o
obj-$(CONFIG_ENV_IS_IN_UBI) += env_ubi.o
obj-$(CONFIG_ENV_IS_NOWHERE) += env_nowhere.o

obj-$(CONFIG_CMD_BEDBUG) += bedbug.o
obj-$(CONFIG_$(SPL_)OF_LIBFDT) += fdt_support.o

obj-$(CONFIG_MII) += miiphyutil.o
obj-$(CONFIG_CMD_MII) += miiphyutil.o
obj-$(CONFIG_PHYLIB) += miiphyutil.o

ifdef CONFIG_CMD_USB
obj-y += usb.o usb_hub.o
obj-$(CONFIG_USB_STORAGE) += usb_storage.o
endif

# others
obj-$(CONFIG_BOOTSTAGE) += bootstage.o
obj-$(CONFIG_CONSOLE_MUX) += iomux.o
obj-y += flash.o
obj-$(CONFIG_CMD_KGDB) += kgdb.o kgdb_stubs.o
obj-$(CONFIG_I2C_EDID) += edid.o
obj-$(CONFIG_KALLSYMS) += kallsyms.o
obj-y += splash.o
obj-$(CONFIG_SPLASH_SOURCE) += splash_source.o
ifndef CONFIG_DM_VIDEO
obj-$(CONFIG_LCD) += lcd.o lcd_console.o
endif
obj-$(CONFIG_LCD_ROTATION) += lcd_console_rotation.o
obj-$(CONFIG_LCD_DT_SIMPLEFB) += lcd_simplefb.o
obj-$(CONFIG_LYNXKDI) += lynxkdi.o
obj-$(CONFIG_MENU) += menu.o
obj-$(CONFIG_CMD_SATA) += sata.o
obj-$(CONFIG_SCSI) += scsi.o
obj-$(CONFIG_UPDATE_TFTP) += update.o
obj-$(CONFIG_DFU_TFTP) += update.o
obj-$(CONFIG_USB_KEYBOARD) += usb_kbd.o

endif # !CONFIG_SPL_BUILD

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_DFU_SUPPORT) += dfu.o
obj-$(CONFIG_SPL_DFU_SUPPORT) += cli_hush.o
obj-$(CONFIG_SPL_HASH_SUPPORT) += hash.o
obj-$(CONFIG_ENV_IS_IN_FLASH) += env_flash.o
obj-$(CONFIG_SPL_YMODEM_SUPPORT) += xyzModem.o
obj-$(CONFIG_SPL_NET_SUPPORT) += miiphyutil.o
obj-$(CONFIG_SPL_OF_TRANSLATE) += fdt_support.o
ifdef CONFIG_SPL_USB_HOST_SUPPORT
obj-$(CONFIG_SPL_USB_SUPPORT) += usb.o usb_hub.o
obj-$(CONFIG_USB_STORAGE) += usb_storage.o
endif
# environment
ifdef CONFIG_TPL_BUILD
obj-$(CONFIG_TPL_ENV_SUPPORT) += env_attr.o
obj-$(CONFIG_TPL_ENV_SUPPORT) += env_flags.o
obj-$(CONFIG_TPL_ENV_SUPPORT) += env_callback.o
else
obj-$(CONFIG_SPL_ENV_SUPPORT) += env_attr.o
obj-$(CONFIG_SPL_ENV_SUPPORT) += env_flags.o
obj-$(CONFIG_SPL_ENV_SUPPORT) += env_callback.o
endif
ifneq ($(CONFIG_TPL_ENV_SUPPORT)$(CONFIG_SPL_ENV_SUPPORT),)
obj-$(CONFIG_ENV_IS_NOWHERE) += env_nowhere.o
obj-$(CONFIG_ENV_IS_IN_MMC) += env_mmc.o
obj-$(CONFIG_ENV_IS_IN_FAT) += env_fat.o
obj-$(CONFIG_ENV_IS_IN_EXT4) += env_ext4.o
obj-$(CONFIG_ENV_IS_IN_NAND) += env_nand.o
obj-$(CONFIG_ENV_IS_IN_SPI_FLASH) += env_sf.o
obj-$(CONFIG_ENV_IS_IN_FLASH) += env_flash.o
endif
ifdef CONFIG_SPL_SATA_SUPPORT
obj-$(CONFIG_SCSI) += scsi.o
endif
endif
#environment
obj-y += env_common.o
#others
obj-$(CONFIG_DDR_SPD) += ddr_spd.o
obj-$(CONFIG_SPD_EEPROM) += ddr_spd.o
obj-$(CONFIG_HWCONFIG) += hwconfig.o
obj-$(CONFIG_BOUNCE_BUFFER) += bouncebuf.o
ifdef CONFIG_SPL_BUILD
ifdef CONFIG_TPL_BUILD
obj-$(CONFIG_TPL_SERIAL_SUPPORT) += console.o
else
obj-$(CONFIG_SPL_SERIAL_SUPPORT) += console.o
endif
else
obj-y += console.o
endif
obj-$(CONFIG_CROS_EC) += cros_ec.o
obj-y += dlmalloc.o
ifdef CONFIG_SYS_MALLOC_F_LEN
obj-y += malloc_simple.o
endif
obj-$(CONFIG_CMD_IDE) += ide.o
obj-y += image.o
obj-$(CONFIG_ANDROID_BOOT_IMAGE) += image-android.o
obj-$(CONFIG_$(SPL_)OF_LIBFDT) += image-fdt.o
obj-$(CONFIG_$(SPL_)FIT) += image-fit.o
obj-$(CONFIG_$(SPL_)FIT_SIGNATURE) += image-sig.o
obj-$(CONFIG_IO_TRACE) += iotrace.o
obj-y += memsize.o
obj-y += stdio.o

# This option is not just y/n - it can have a numeric value
ifdef CONFIG_FASTBOOT_FLASH
obj-y += image-sparse.o
ifdef CONFIG_FASTBOOT_FLASH_MMC_DEV
obj-y += fb_mmc.o
endif
ifdef CONFIG_FASTBOOT_FLASH_NAND_DEV
obj-y += fb_nand.o
endif
endif

ifdef CONFIG_CMD_EEPROM_LAYOUT
obj-y += eeprom/eeprom_field.o eeprom/eeprom_layout.o
endif
# We always have this since drivers/ddr/fs/interactive.c needs it
obj-$(CONFIG_CMDLINE) += cli_simple.o

obj-y += cli.o
obj-$(CONFIG_CMDLINE) += cli_readline.o
obj-$(CONFIG_CMD_DFU) += dfu.o
obj-y += command.o
obj-y += s_record.o
obj-y += xyzModem.o

CFLAGS_env_embedded.o := -Wa,--no-warn -DENV_CRC=$(shell tools/envcrc 2>/dev/null)
