#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SPL_BUILD
# core command
obj-y += cmd_boot.o
obj-$(CONFIG_CMD_BOOTM) += cmd_bootm.o
obj-y += cmd_help.o
obj-y += cmd_version.o

# command
obj-$(CONFIG_CMD_AES) += cmd_aes.o
obj-$(CONFIG_CMD_AMBAPP) += cmd_ambapp.o
obj-$(CONFIG_CMD_ARMFLASH) += cmd_armflash.o
obj-$(CONFIG_SOURCE) += cmd_source.o
obj-$(CONFIG_CMD_SOURCE) += cmd_source.o
obj-$(CONFIG_CMD_BDI) += cmd_bdinfo.o
obj-$(CONFIG_CMD_BEDBUG) += cmd_bedbug.o
obj-$(CONFIG_CMD_BMP) += cmd_bmp.o
obj-$(CONFIG_CMD_BOOTMENU) += cmd_bootmenu.o
obj-$(CONFIG_CMD_BOOTLDR) += cmd_bootldr.o
obj-$(CONFIG_CMD_BOOTSTAGE) += cmd_bootstage.o
obj-$(CONFIG_CMD_CACHE) += cmd_cache.o
obj-$(CONFIG_CMD_CBFS) += cmd_cbfs.o
obj-$(CONFIG_CMD_CLK) += cmd_clk.o
obj-$(CONFIG_CMD_CONSOLE) += cmd_console.o
obj-$(CONFIG_CMD_CPLBINFO) += cmd_cplbinfo.o
obj-$(CONFIG_CMD_CPU) += cmd_cpu.o
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
obj-$(CONFIG_EFI_STUB) += cmd_efi.o
obj-$(CONFIG_CMD_ELF) += cmd_elf.o
obj-$(CONFIG_SYS_HUSH_PARSER) += cmd_exit.o
obj-$(CONFIG_CMD_EXT4) += cmd_ext4.o
obj-$(CONFIG_CMD_EXT2) += cmd_ext2.o
obj-$(CONFIG_CMD_FAT) += cmd_fat.o
obj-$(CONFIG_CMD_FDC) += cmd_fdc.o
obj-$(CONFIG_OF_LIBFDT) += cmd_fdt.o
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
obj-$(CONFIG_CMD_IOTRACE) += cmd_iotrace.o
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
obj-$(CONFIG_CMD_REMOTEPROC) += cmd_remoteproc.o
obj-$(CONFIG_SANDBOX) += cmd_host.o
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
obj-$(CONFIG_CMD_TPM_TEST) += cmd_tpm_test.o
obj-$(CONFIG_CMD_TSI148) += cmd_tsi148.o
obj-$(CONFIG_CMD_UBI) += cmd_ubi.o
obj-$(CONFIG_CMD_UBIFS) += cmd_ubifs.o
obj-$(CONFIG_CMD_UNIVERSE) += cmd_universe.o
obj-$(CONFIG_CMD_UNZIP) += cmd_unzip.o
ifdef CONFIG_LZMA
obj-$(CONFIG_CMD_LZMADEC) += cmd_lzmadec.o
endif

obj-$(CONFIG_CMD_USB) += cmd_usb.o
obj-$(CONFIG_CMD_FASTBOOT) += cmd_fastboot.o
obj-$(CONFIG_CMD_FS_UUID) += cmd_fs_uuid.o

obj-$(CONFIG_CMD_USB_MASS_STORAGE) += cmd_usb_mass_storage.o
obj-$(CONFIG_CMD_THOR_DOWNLOAD) += cmd_thordown.o
obj-$(CONFIG_CMD_XIMG) += cmd_ximg.o
obj-$(CONFIG_YAFFS2) += cmd_yaffs2.o
obj-$(CONFIG_CMD_SPL) += cmd_spl.o
obj-$(CONFIG_CMD_ZIP) += cmd_zip.o
obj-$(CONFIG_CMD_ZFS) += cmd_zfs.o

obj-$(CONFIG_CMD_DFU) += cmd_dfu.o
obj-$(CONFIG_CMD_GPT) += cmd_gpt.o
obj-$(CONFIG_CMD_ETHSW) += cmd_ethsw.o

# Power
obj-$(CONFIG_CMD_PMIC) += cmd_pmic.o
obj-$(CONFIG_CMD_REGULATOR) += cmd_regulator.o
endif # !CONFIG_SPL_BUILD

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_SATA_SUPPORT
obj-$(CONFIG_CMD_SCSI) += cmd_scsi.o
endif
endif # CONFIG_SPL_BUILD

obj-$(CONFIG_CMD_BLOB) += cmd_blob.o

# core command
obj-y += cmd_nvedit.o
obj-y += cmd_disk.o
