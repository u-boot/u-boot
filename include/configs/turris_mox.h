/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marek Behun <marek.behun@nic.cz>
 *
 * Based on mvebu_armada-37xx.h by Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_TURRIS_MOX_H
#define _CONFIG_TURRIS_MOX_H

#define CONFIG_SYS_BOOTM_LEN (64 << 20)

#define CONFIG_LAST_STAGE_INIT

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_DISPLAY_BOARDINFO_LATE

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

/* auto boot */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, \
					  115200, 230400, 460800, 921600 }

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG		/* enable INITRD tag */
#define CONFIG_SETUP_MEMORY_TAGS	/* enable memory tag */

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(4 << 20) /* 4MiB for malloc() */

/*
 * Other required minimal configurations
 */
#define CONFIG_SYS_LOAD_ADDR	0x00800000	/* default load adr- 8M */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */

/* End of 16M scrubbed by training in bootrom */
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_TEXT_BASE + 0xFF0000)

/*
 * I2C
 */
#define CONFIG_I2C_MV
#define CONFIG_SYS_I2C_SLAVE		0x0

/* Environment in SPI NOR flash */

/*
 * Ethernet Driver configuration
 */
#define CONFIG_ARP_TIMEOUT	200
#define CONFIG_NET_RETRY_COUNT	50

#define CONFIG_USB_MAX_CONTROLLER_COUNT (3 + 3)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define TURRIS_MOX_BOOTCMD_RESCUE \
	"setenv bootargs \"console=ttyMV0,115200 " \
			  "earlycon=ar3700_uart,0xd0012000\" && " \
	"sf probe && " \
	"sf read 0x5000000 0x190000 && " \
	"lzmadec 0x5000000 0x5800000 && " \
	"bootm 0x5800000"

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x4d00000\0"	\
	"pxefile_addr_r=0x4e00000\0"	\
	"fdt_addr_r=0x4f00000\0"	\
	"kernel_addr_r=0x5000000\0"	\
	"ramdisk_addr_r=0x8000000\0"	\
	"fdtfile=marvell/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"bootcmd_rescue=" TURRIS_MOX_BOOTCMD_RESCUE "\0" \
	BOOTENV

#endif /* _CONFIG_TURRIS_MOX_H */
