/*
 * Board configuration file for Dragonboard 410C
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIGS_DRAGONBOARD410C_H
#define __CONFIGS_DRAGONBOARD410C_H

#include <linux/sizes.h>
#include <asm/arch/sysmap-apq8016.h>

#define CONFIG_MISC_INIT_R /* To stop autoboot */

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x80000000
/* 1008 MB (the last ~30Mb are secured for TrustZone by ATF*/
#define PHYS_SDRAM_1_SIZE		0x3da00000
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_TEXT_BASE		0x80080000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x80000)
#define CONFIG_SYS_BOOTM_LEN		0x1000000 /* 16MB max kernel size */

/* UART */

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		19000000

#define CONFIG_SYS_LDSCRIPT "board/qualcomm/dragonboard410c/u-boot.lds"

/* Fixup - in init code we switch from device to host mode,
 * it has to be done after each HCD reset */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

#define CONFIG_USB_HOST_ETHER /* Enable USB Networking */

/* Support all possible USB ethernet dongles */
#define CONFIG_USB_ETHER_DM9601
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_ASIX88179
#define CONFIG_USB_ETHER_MCS7830
#define CONFIG_USB_ETHER_SMSC95XX

/* Extra Commands */
/* Enable that for switching of boot partitions */
/* Disabled by default as some sub-commands can brick eMMC */
/*#define CONFIG_SUPPORT_EMMC_BOOT */
#define CONFIG_CMD_REGINFO	/* Register dump		*/
#define CONFIG_CMD_TFTP

/* Partition table support */
#define HAVE_BLOCK_DEVICE /* Needed for partition commands */

#include <config_distro_defaults.h>

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Environment - Boot*/
#define CONFIG_BOOTARGS "console=ttyMSM0,115200n8"

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Does what recovery does */
#define REFLASH(file, part) \
"part start mmc 0 "#part" start && "\
"part size mmc 0 "#part" size && "\
"tftp $loadaddr "#file" && " \
"mmc write $loadaddr $start $size && "

#define CONFIG_ENV_REFLASH \
"mmc dev 0 && "\
"usb start && "\
"dhcp && "\
"tftp $loadaddr dragonboard/rescue/gpt_both0.bin && "\
"mmc write $loadaddr 0 43 && " \
"mmc rescan && "\
REFLASH(dragonboard/rescue/NON-HLOS.bin, 1)\
REFLASH(dragonboard/rescue/sbl1.mbn, 2)\
REFLASH(dragonboard/rescue/rpm.mbn, 3)\
REFLASH(dragonboard/rescue/tz.mbn, 4)\
REFLASH(dragonboard/rescue/hyp.mbn, 5)\
REFLASH(dragonboard/rescue/sec.dat, 6)\
REFLASH(dragonboard/rescue/emmc_appsboot.mbn, 7)\
REFLASH(dragonboard/u-boot.img, 8)\
"usb stop &&"\
"echo Reflash completed"

/* Environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"reflash="CONFIG_ENV_REFLASH"\0"\
	"loadaddr=0x81000000\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"linux_image=Image\0" \
	"kernel_addr_r=0x81000000\0"\
	"fdtfile=apq8016-sbc.dtb\0" \
	"fdt_addr_r=0x83000000\0"\
	"ramdisk_addr_r=0x84000000\0"\
	"scriptaddr=0x90000000\0"\
	"pxefile_addr_r=0x90100000\0"\
	BOOTENV

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_VARS_UBOOT_CONFIG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + SZ_8M)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#endif
