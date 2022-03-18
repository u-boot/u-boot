/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_MVEBU_ARMADA_8K_H
#define _CONFIG_MVEBU_ARMADA_8K_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

/* auto boot */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, \
					  115200, 230400, 460800, 921600 }

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */

/*
 * Other required minimal configurations
 */
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */

/* End of 16M scrubbed by training in bootrom */
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_TEXT_BASE + 0xFF0000)

/* When runtime detection fails this is the default */

#define CONFIG_SYS_MAX_NAND_DEVICE	1

#define CONFIG_USB_MAX_CONTROLLER_COUNT (3 + 3)

/* USB ethernet */

/*
 * SATA/SCSI/AHCI configuration
 */
#define CONFIG_LBA48
#define CONFIG_SYS_64BIT_LBA

/*
 * PCI configuration
 */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x6d00000\0"	\
	"pxefile_addr_r=0x6e00000\0"	\
	"fdt_addr_r=0x6f00000\0"	\
	"kernel_addr_r=0x7000000\0"	\
	"ramdisk_addr_r=0xa000000\0"	\
	"fdtfile=marvell/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	BOOTENV

#endif /* _CONFIG_MVEBU_ARMADA_8K_H */
