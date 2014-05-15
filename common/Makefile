#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# core
ifndef CONFIG_SPL_BUILD
obj-y += main.o
obj-y += command.o
obj-y += exports.o
obj-y += hash.o
obj-$(CONFIG_SYS_HUSH_PARSER) += hush.o
obj-y += s_record.o
obj-y += xyzModem.o
obj-y += cmd_disk.o

# boards
obj-$(CONFIG_SYS_GENERIC_BOARD) += board_f.o
obj-$(CONFIG_SYS_GENERIC_BOARD) += board_r.o

# core command
obj-y += cmd_boot.o
obj-$(CONFIG_CMD_BOOTM) += cmd_bootm.o
obj-y += cmd_help.o
obj-y += cmd_version.o

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
obj-$(CONFIG_ENV_IS_IN_NAND) += env_nand.o
obj-$(CONFIG_ENV_IS_IN_NVRAM) += env_nvram.o
obj-$(CONFIG_ENV_IS_IN_ONENAND) += env_onenand.o
obj-$(CONFIG_ENV_IS_IN_SPI_FLASH) += env_sf.o
obj-$(CONFIG_ENV_IS_IN_REMOTE) += env_remote.o
obj-$(CONFIG_ENV_IS_IN_UBI) += env_ubi.o
obj-$(CONFIG_ENV_IS_NOWHERE) += env_nowhere.o

# command
obj-$(CONFIG_CMD_AES) += cmd_aes.o
obj-$(CONFIG_CMD_AMBAPP) += cmd_ambapp.o
obj-$(CONFIG_SOURCE) += cmd_source.o
obj-$(CONFIG_CMD_SOURCE) += cmd_source.o
obj-$(CONFIG_CMD_BDI) += cmd_bdinfo.o
obj-$(CONFIG_CMD_BEDBUG) += bedbug.o cmd_bedbug.o
obj-$(CONFIG_CMD_BMP) += cmd_bmp.o
obj-$(CONFIG_CMD_BOOTMENU) += cmd_bootmenu.o
obj-$(CONFIG_CMD_BOOTLDR) += cmd_bootldr.o
obj-$(CONFIG_CMD_BOOTSTAGE) += cmd_bootstage.o
obj-$(CONFIG_CMD_CACHE) += cmd_cache.o
obj-$(CONFIG_CMD_CBFS) += cmd_cbfs.o
obj-$(CONFIG_CMD_CLK) += cmd_clk.o
obj-$(CONFIG_CMD_CONSOLE) += cmd_console.o
obj-$(CONFIG_CMD_CPLBINFO) += cmd_cplbinfo.o
obj-$(CONFIG_DATAFLASH_MMC_SELECT) += cmd_dataflash_mmc_mux.o
obj-$(CONFIG_CMD_DATE) += cmd_date.o
obj-$(CONFIG_CMD_DEMO) += cmd_demo.o
obj-$(CONFIG_CMD_SOUND) += cmd_sound.o
ifdef CONFIG_4xx
obj-$(CONFIG_CMD_SETGETDCR) += cmd_dcr.o
endif
ifdef CONFIG_POST
obj-$(CONFIG_CMD_DIAG) += cmd_diag.o
endif
obj-$(CONFIG_CMD_DISPLAY) += cmd_display.o
obj-$(CONFIG_CMD_DTT) += cmd_dtt.o
obj-$(CONFIG_CMD_ECHO) += cmd_echo.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += cmd_eeprom.o
obj-$(CONFIG_CMD_EEPROM) += cmd_eeprom.o
obj-$(CONFIG_CMD_ELF) += cmd_elf.o
obj-$(CONFIG_SYS_HUSH_PARSER) += cmd_exit.o
obj-$(CONFIG_CMD_EXT4) += cmd_ext4.o
obj-$(CONFIG_CMD_EXT2) += cmd_ext2.o
obj-$(CONFIG_CMD_FAT) += cmd_fat.o
obj-$(CONFIG_CMD_FDC) += cmd_fdc.o
obj-$(CONFIG_OF_LIBFDT) += cmd_fdt.o fdt_support.o
obj-$(CONFIG_CMD_FITUPD) += cmd_fitupd.o
obj-$(CONFIG_CMD_FLASH) += cmd_flash.o
ifdef CONFIG_FPGA
obj-$(CONFIG_CMD_FPGA) += cmd_fpga.o
endif
obj-$(CONFIG_CMD_FPGAD) += cmd_fpgad.o
obj-$(CONFIG_CMD_FS_GENERIC) += cmd_fs.o
obj-$(CONFIG_CMD_FUSE) += cmd_fuse.o
obj-$(CONFIG_CMD_GETTIME) += cmd_gettime.o
obj-$(CONFIG_CMD_GPIO) += cmd_gpio.o
obj-$(CONFIG_CMD_I2C) += cmd_i2c.o
obj-$(CONFIG_CMD_HASH) += cmd_hash.o
obj-$(CONFIG_CMD_IDE) += cmd_ide.o
obj-$(CONFIG_CMD_IMMAP) += cmd_immap.o
obj-$(CONFIG_CMD_INI) += cmd_ini.o
obj-$(CONFIG_CMD_IRQ) += cmd_irq.o
obj-$(CONFIG_CMD_ITEST) += cmd_itest.o
obj-$(CONFIG_CMD_JFFS2) += cmd_jffs2.o
obj-$(CONFIG_CMD_CRAMFS) += cmd_cramfs.o
obj-$(CONFIG_CMD_LDRINFO) += cmd_ldrinfo.o
obj-$(CONFIG_CMD_LED) += cmd_led.o
obj-$(CONFIG_CMD_LICENSE) += cmd_license.o
obj-y += cmd_load.o
obj-$(CONFIG_LOGBUFFER) += cmd_log.o
obj-$(CONFIG_ID_EEPROM) += cmd_mac.o
obj-$(CONFIG_CMD_MD5SUM) += cmd_md5sum.o
obj-$(CONFIG_CMD_MEMORY) += cmd_mem.o
obj-$(CONFIG_CMD_IO) += cmd_io.o
obj-$(CONFIG_CMD_MFSL) += cmd_mfsl.o
obj-$(CONFIG_MII) += miiphyutil.o
obj-$(CONFIG_CMD_MII) += miiphyutil.o
obj-$(CONFIG_PHYLIB) += miiphyutil.o
obj-$(CONFIG_CMD_MII) += cmd_mii.o
ifdef CONFIG_PHYLIB
obj-$(CONFIG_CMD_MII) += cmd_mdio.o
endif
obj-$(CONFIG_CMD_MISC) += cmd_misc.o
obj-$(CONFIG_CMD_MMC) += cmd_mmc.o
obj-$(CONFIG_CMD_MMC_SPI) += cmd_mmc_spi.o
obj-$(CONFIG_MP) += cmd_mp.o
obj-$(CONFIG_CMD_MTDPARTS) += cmd_mtdparts.o
obj-$(CONFIG_CMD_NAND) += cmd_nand.o
obj-$(CONFIG_CMD_NET) += cmd_net.o
obj-$(CONFIG_CMD_ONENAND) += cmd_onenand.o
obj-$(CONFIG_CMD_OTP) += cmd_otp.o
obj-$(CONFIG_CMD_PART) += cmd_part.o
ifdef CONFIG_PCI
obj-$(CONFIG_CMD_PCI) += cmd_pci.o
endif
obj-y += cmd_pcmcia.o
obj-$(CONFIG_CMD_PORTIO) += cmd_portio.o
obj-$(CONFIG_CMD_PXE) += cmd_pxe.o
obj-$(CONFIG_CMD_READ) += cmd_read.o
obj-$(CONFIG_CMD_REGINFO) += cmd_reginfo.o
obj-$(CONFIG_CMD_REISER) += cmd_reiser.o
obj-$(CONFIG_SANDBOX) += cmd_sandbox.o
obj-$(CONFIG_CMD_SATA) += cmd_sata.o
obj-$(CONFIG_CMD_SF) += cmd_sf.o
obj-$(CONFIG_CMD_SCSI) += cmd_scsi.o
obj-$(CONFIG_CMD_SHA1SUM) += cmd_sha1sum.o
obj-$(CONFIG_CMD_SETEXPR) += cmd_setexpr.o
obj-$(CONFIG_CMD_SOFTSWITCH) += cmd_softswitch.o
obj-$(CONFIG_CMD_SPI) += cmd_spi.o
obj-$(CONFIG_CMD_SPIBOOTLDR) += cmd_spibootldr.o
obj-$(CONFIG_CMD_STRINGS) += cmd_strings.o
obj-$(CONFIG_CMD_TERMINAL) += cmd_terminal.o
obj-$(CONFIG_CMD_TIME) += cmd_time.o
obj-$(CONFIG_CMD_TRACE) += cmd_trace.o
obj-$(CONFIG_SYS_HUSH_PARSER) += cmd_test.o
obj-$(CONFIG_CMD_TPM) += cmd_tpm.o
obj-$(CONFIG_CMD_TSI148) += cmd_tsi148.o
obj-$(CONFIG_CMD_UBI) += cmd_ubi.o
obj-$(CONFIG_CMD_UBIFS) += cmd_ubifs.o
obj-$(CONFIG_CMD_UNIVERSE) += cmd_universe.o
obj-$(CONFIG_CMD_UNZIP) += cmd_unzip.o
ifdef CONFIG_LZMA
obj-$(CONFIG_CMD_LZMADEC) += cmd_lzmadec.o
endif
ifdef CONFIG_CMD_USB
obj-y += cmd_usb.o
obj-y += usb.o usb_hub.o
obj-$(CONFIG_USB_STORAGE) += usb_storage.o
endif
obj-$(CONFIG_CMD_USB_MASS_STORAGE) += cmd_usb_mass_storage.o
obj-$(CONFIG_CMD_THOR_DOWNLOAD) += cmd_thordown.o
obj-$(CONFIG_CMD_XIMG) += cmd_ximg.o
obj-$(CONFIG_YAFFS2) += cmd_yaffs2.o
obj-$(CONFIG_CMD_SPL) += cmd_spl.o
obj-$(CONFIG_CMD_ZIP) += cmd_zip.o
obj-$(CONFIG_CMD_ZFS) += cmd_zfs.o

# others
obj-$(CONFIG_BOOTSTAGE) += bootstage.o
obj-$(CONFIG_CONSOLE_MUX) += iomux.o
obj-y += flash.o
obj-$(CONFIG_CMD_KGDB) += kgdb.o kgdb_stubs.o
obj-$(CONFIG_I2C_EDID) += edid.o
obj-$(CONFIG_KALLSYMS) += kallsyms.o
obj-y += splash.o
obj-$(CONFIG_LCD) += lcd.o
obj-$(CONFIG_LYNXKDI) += lynxkdi.o
obj-$(CONFIG_MENU) += menu.o
obj-$(CONFIG_MODEM_SUPPORT) += modem.o
obj-$(CONFIG_UPDATE_TFTP) += update.o
obj-$(CONFIG_USB_KEYBOARD) += usb_kbd.o
obj-$(CONFIG_CMD_DFU) += cmd_dfu.o
obj-$(CONFIG_CMD_GPT) += cmd_gpt.o
endif

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ENV_IS_IN_FLASH) += env_flash.o
obj-$(CONFIG_SPL_YMODEM_SUPPORT) += xyzModem.o
obj-$(CONFIG_SPL_NET_SUPPORT) += miiphyutil.o
# environment
obj-$(CONFIG_SPL_ENV_SUPPORT) += env_attr.o
obj-$(CONFIG_SPL_ENV_SUPPORT) += env_flags.o
obj-$(CONFIG_SPL_ENV_SUPPORT) += env_callback.o
ifdef CONFIG_SPL_USB_HOST_SUPPORT
obj-$(CONFIG_SPL_USB_SUPPORT) += usb.o usb_hub.o
obj-$(CONFIG_USB_STORAGE) += usb_storage.o
endif
ifdef CONFIG_SPL_SATA_SUPPORT
obj-$(CONFIG_CMD_SCSI) += cmd_scsi.o
endif
ifneq ($(CONFIG_SPL_NET_SUPPORT),y)
obj-$(CONFIG_ENV_IS_NOWHERE) += env_nowhere.o
obj-$(CONFIG_ENV_IS_IN_MMC) += env_mmc.o
obj-$(CONFIG_ENV_IS_IN_NAND) += env_nand.o
obj-$(CONFIG_ENV_IS_IN_SPI_FLASH) += env_sf.o
obj-$(CONFIG_ENV_IS_IN_FLASH) += env_flash.o
else
obj-y += env_nowhere.o
endif
endif
# core command
obj-y += cmd_nvedit.o
#environment
obj-y += env_common.o
#others
ifdef CONFIG_DDR_SPD
SPD := y
endif
ifdef CONFIG_SPD_EEPROM
SPD := y
endif
obj-$(SPD) += ddr_spd.o
obj-$(CONFIG_HWCONFIG) += hwconfig.o
obj-$(CONFIG_BOUNCE_BUFFER) += bouncebuf.o
obj-y += console.o
obj-$(CONFIG_CROS_EC) += cros_ec.o
obj-y += dlmalloc.o
obj-y += image.o
obj-$(CONFIG_OF_LIBFDT) += image-fdt.o
obj-$(CONFIG_FIT) += image-fit.o
obj-$(CONFIG_FIT_SIGNATURE) += image-sig.o
obj-y += memsize.o
obj-y += stdio.o

CFLAGS_env_embedded.o := -Wa,--no-warn -DENV_CRC=$(shell tools/envcrc 2>/dev/null)
