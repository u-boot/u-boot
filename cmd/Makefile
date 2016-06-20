#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SPL_BUILD
# core command
obj-y += boot.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-y += help.o
obj-y += version.o

# command
obj-$(CONFIG_CMD_AES) += aes.o
obj-$(CONFIG_CMD_AMBAPP) += ambapp.o
obj-$(CONFIG_CMD_ARMFLASH) += armflash.o
obj-$(CONFIG_SOURCE) += source.o
obj-$(CONFIG_CMD_SOURCE) += source.o
obj-$(CONFIG_CMD_BDI) += bdinfo.o
obj-$(CONFIG_CMD_BEDBUG) += bedbug.o
obj-$(CONFIG_CMD_BLOCK_CACHE) += blkcache.o
obj-$(CONFIG_CMD_BMP) += bmp.o
obj-$(CONFIG_CMD_BOOTEFI) += bootefi.o
obj-$(CONFIG_CMD_BOOTMENU) += bootmenu.o
obj-$(CONFIG_CMD_BOOTLDR) += bootldr.o
obj-$(CONFIG_CMD_BOOTSTAGE) += bootstage.o
obj-$(CONFIG_CMD_CACHE) += cache.o
obj-$(CONFIG_CMD_CBFS) += cbfs.o
obj-$(CONFIG_CMD_CLK) += clk.o
obj-$(CONFIG_CMD_CONSOLE) += console.o
obj-$(CONFIG_CMD_CPLBINFO) += cplbinfo.o
obj-$(CONFIG_CMD_CPU) += cpu.o
obj-$(CONFIG_DATAFLASH_MMC_SELECT) += dataflash_mmc_mux.o
obj-$(CONFIG_CMD_DATE) += date.o
obj-$(CONFIG_CMD_DEMO) += demo.o
obj-$(CONFIG_CMD_SOUND) += sound.o
ifdef CONFIG_4xx
obj-$(CONFIG_CMD_SETGETDCR) += dcr.o
endif
ifdef CONFIG_POST
obj-$(CONFIG_CMD_DIAG) += diag.o
endif
obj-$(CONFIG_CMD_DISPLAY) += display.o
obj-$(CONFIG_CMD_DTT) += dtt.o
obj-$(CONFIG_CMD_ECHO) += echo.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += eeprom.o
obj-$(CONFIG_CMD_EEPROM) += eeprom.o
obj-$(CONFIG_EFI_STUB) += efi.o
obj-$(CONFIG_CMD_ELF) += elf.o
obj-$(CONFIG_HUSH_PARSER) += exit.o
obj-$(CONFIG_CMD_EXT4) += ext4.o
obj-$(CONFIG_CMD_EXT2) += ext2.o
obj-$(CONFIG_CMD_FAT) += fat.o
obj-$(CONFIG_CMD_FDC) += fdc.o
obj-$(CONFIG_CMD_FDT) += fdt.o
obj-$(CONFIG_CMD_FITUPD) += fitupd.o
obj-$(CONFIG_CMD_FLASH) += flash.o
ifdef CONFIG_FPGA
obj-$(CONFIG_CMD_FPGA) += fpga.o
endif
obj-$(CONFIG_CMD_FPGAD) += fpgad.o
obj-$(CONFIG_CMD_FS_GENERIC) += fs.o
obj-$(CONFIG_CMD_FUSE) += fuse.o
obj-$(CONFIG_CMD_GETTIME) += gettime.o
obj-$(CONFIG_CMD_GPIO) += gpio.o
obj-$(CONFIG_CMD_I2C) += i2c.o
obj-$(CONFIG_CMD_IOTRACE) += iotrace.o
obj-$(CONFIG_CMD_HASH) += hash.o
obj-$(CONFIG_CMD_IDE) += ide.o
obj-$(CONFIG_CMD_IMMAP) += immap.o
obj-$(CONFIG_CMD_INI) += ini.o
obj-$(CONFIG_CMD_IRQ) += irq.o
obj-$(CONFIG_CMD_ITEST) += itest.o
obj-$(CONFIG_CMD_JFFS2) += jffs2.o
obj-$(CONFIG_CMD_CRAMFS) += cramfs.o
obj-$(CONFIG_CMD_LDRINFO) += ldrinfo.o
obj-$(CONFIG_CMD_LED) += led.o
obj-$(CONFIG_CMD_LICENSE) += license.o
obj-y += load.o
obj-$(CONFIG_LOGBUFFER) += log.o
obj-$(CONFIG_ID_EEPROM) += mac.o
obj-$(CONFIG_CMD_MD5SUM) += md5sum.o
obj-$(CONFIG_CMD_MEMORY) += mem.o
obj-$(CONFIG_CMD_IO) += io.o
obj-$(CONFIG_CMD_MFSL) += mfsl.o
obj-$(CONFIG_CMD_MII) += mii.o
ifdef CONFIG_PHYLIB
obj-$(CONFIG_CMD_MII) += mdio.o
endif
obj-$(CONFIG_CMD_MISC) += misc.o
obj-$(CONFIG_CMD_MMC) += mmc.o
obj-$(CONFIG_CMD_MMC_SPI) += mmc_spi.o
obj-$(CONFIG_MP) += mp.o
obj-$(CONFIG_CMD_MTDPARTS) += mtdparts.o
obj-$(CONFIG_CMD_NAND) += nand.o
obj-$(CONFIG_CMD_NET) += net.o
obj-$(CONFIG_CMD_ONENAND) += onenand.o
obj-$(CONFIG_CMD_OTP) += otp.o
obj-$(CONFIG_CMD_PART) += part.o
ifdef CONFIG_PCI
obj-$(CONFIG_CMD_PCI) += pci.o
endif
obj-y += pcmcia.o
obj-$(CONFIG_CMD_PORTIO) += portio.o
obj-$(CONFIG_CMD_PXE) += pxe.o
obj-$(CONFIG_CMD_QFW) += qfw.o
obj-$(CONFIG_CMD_READ) += read.o
obj-$(CONFIG_CMD_REGINFO) += reginfo.o
obj-$(CONFIG_CMD_REISER) += reiser.o
obj-$(CONFIG_CMD_REMOTEPROC) += remoteproc.o
obj-$(CONFIG_SANDBOX) += host.o
obj-$(CONFIG_CMD_SATA) += sata.o
obj-$(CONFIG_CMD_SF) += sf.o
obj-$(CONFIG_SCSI) += scsi.o
obj-$(CONFIG_CMD_SHA1SUM) += sha1sum.o
obj-$(CONFIG_CMD_SETEXPR) += setexpr.o
obj-$(CONFIG_CMD_SOFTSWITCH) += softswitch.o
obj-$(CONFIG_CMD_SPI) += spi.o
obj-$(CONFIG_CMD_SPIBOOTLDR) += spibootldr.o
obj-$(CONFIG_CMD_STRINGS) += strings.o
obj-$(CONFIG_CMD_TERMINAL) += terminal.o
obj-$(CONFIG_CMD_TIME) += time.o
obj-$(CONFIG_CMD_TRACE) += trace.o
obj-$(CONFIG_HUSH_PARSER) += test.o
obj-$(CONFIG_CMD_TPM) += tpm.o
obj-$(CONFIG_CMD_TPM_TEST) += tpm_test.o
obj-$(CONFIG_CMD_TSI148) += tsi148.o
obj-$(CONFIG_CMD_UBI) += ubi.o
obj-$(CONFIG_CMD_UBIFS) += ubifs.o
obj-$(CONFIG_CMD_UNIVERSE) += universe.o
obj-$(CONFIG_CMD_UNZIP) += unzip.o
ifdef CONFIG_LZMA
obj-$(CONFIG_CMD_LZMADEC) += lzmadec.o
endif

obj-$(CONFIG_CMD_USB) += usb.o
obj-$(CONFIG_CMD_FASTBOOT) += fastboot.o
obj-$(CONFIG_CMD_FS_UUID) += fs_uuid.o

obj-$(CONFIG_CMD_USB_MASS_STORAGE) += usb_mass_storage.o
obj-$(CONFIG_CMD_THOR_DOWNLOAD) += thordown.o
obj-$(CONFIG_CMD_XIMG) += ximg.o
obj-$(CONFIG_YAFFS2) += yaffs2.o
obj-$(CONFIG_CMD_SPL) += spl.o
obj-$(CONFIG_CMD_ZIP) += zip.o
obj-$(CONFIG_CMD_ZFS) += zfs.o

obj-$(CONFIG_CMD_DFU) += dfu.o
obj-$(CONFIG_CMD_GPT) += gpt.o
obj-$(CONFIG_CMD_ETHSW) += ethsw.o

# Power
obj-$(CONFIG_CMD_PMIC) += pmic.o
obj-$(CONFIG_CMD_REGULATOR) += regulator.o
endif # !CONFIG_SPL_BUILD

obj-$(CONFIG_CMD_BLOB) += blob.o

# core command
obj-y += nvedit.o
obj-y += disk.o
