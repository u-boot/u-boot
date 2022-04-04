/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_MVEBU_ARMADA_37XX_H
#define _CONFIG_MVEBU_ARMADA_37XX_H

#include <linux/sizes.h>

/*
 * High Level Configuration Options (easy to change)
 */

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

#define CONFIG_SYS_BOOTM_LEN	SZ_64M /* Increase max gunzip size */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 300, 600, 1200, 1800, 2400, 4800, \
					  9600, 19200, 38400, 57600, 115200, \
					  230400, 460800, 500000, 576000, \
					  921600, 1000000, 1152000, 1500000, \
					  2000000, 2500000, 3000000, 3500000, \
					  4000000, 4500000, 5000000, 5500000, \
					  6000000 }

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */

/*
 * Other required minimal configurations
 */
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */

/* End of 16M scrubbed by training in bootrom */
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_TEXT_BASE + 0xFF0000)

/*
 * Environment
 */
#define DEFAULT_ENV_IS_RW		/* required for configuring default fdtfile= */

#define CONFIG_USB_MAX_CONTROLLER_COUNT (3 + 3)

/*
 * SATA/SCSI/AHCI configuration
 */
#define CONFIG_LBA48
#define CONFIG_SYS_64BIT_LBA

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* filler for default values filled by board_early_init_f() */
#define ENV_RW_FILLER \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for ethaddr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for eth1addr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for eth2addr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for eth3addr= */ \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" /* for fdtfile= */ \
	""

/* fdt_addr and kernel_addr are needed for existing distribution boot scripts */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x6d00000\0"	\
	"pxefile_addr_r=0x6e00000\0"	\
	"fdt_addr=0x6f00000\0"		\
	"fdt_addr_r=0x6f00000\0"	\
	"kernel_addr=0x7000000\0"	\
	"kernel_addr_r=0x7000000\0"	\
	"ramdisk_addr_r=0xa000000\0"	\
	BOOTENV \
	ENV_RW_FILLER

#endif /* _CONFIG_MVEBU_ARMADA_37XX_H */
