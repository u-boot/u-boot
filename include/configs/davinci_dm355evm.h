/*
 * Copyright (C) 2009 David Brownell
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <asm/sizes.h>

/* Spectrum Digital TMS320DM355 EVM board */
#define DAVINCI_DM355EVM

#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is a 3rd stage loader */
#define CONFIG_SKIP_RELOCATE_UBOOT
#define CONFIG_SYS_NO_FLASH		/* that is, no *NOR* flash */
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_DISPLAY_CPUINFO

/* SoC Configuration */
#define CONFIG_ARM926EJS				/* arm926ejs CPU */
#define CONFIG_SYS_TIMERBASE		0x01c21400	/* use timer 0 */
#define CONFIG_SYS_HZ_CLOCK		24000000	/* timer0 freq */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SOC_DM355

/* Memory Info */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		SZ_128M

/* Serial Driver info: UART0 for console  */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM1		0x01c20000
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_HZ_CLOCK
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* Ethernet:  external DM9000 */
#define CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x04014000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE + 2)
#define CONFIG_NET_MULTI

/* I2C */
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_SYS_I2C_SLAVE		0x10	/* SMBus host address */

/* NAND: socketed, two chipselects, normally 2 GBytes */
/* NYET -- #define CONFIG_NAND_DAVINCI */
#define CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_NAND_USE_FLASH_BBT

#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_SYS_NAND_BASE_LIST	{ 0x02000000, }
/* socket has two chipselects, nCE0 gated by address BIT(14) */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	2

/* USB: OTG connector */
/* NYET -- #define CONFIG_USB_DAVINCI */

/* U-Boot command configuration */
#include <config_cmd_default.h>

#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES

#ifdef CONFIG_NAND_DAVINCI
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE
#endif

/* TEMPORARY -- no safe place to save env, yet */
#define CONFIG_ENV_IS_NOWHERE
#undef CONFIG_CMD_SAVEENV

#ifdef CONFIG_USB_DAVINCI
#define CONFIG_MUSB_HCD
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#else
#undef CONFIG_MUSB_HCD
#undef CONFIG_CMD_USB
#undef CONFIG_USB_STORAGE
#endif

#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

/* U-Boot general configuration */
#undef CONFIG_USE_IRQ				/* No IRQ/FIQ in U-Boot */
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CONFIG_SYS_PROMPT	"DM355 EVM # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE			/* Print buffer size */ \
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_LONGHELP

#define CONFIG_ENV_SIZE		SZ_16K

/* NYET -- #define CONFIG_BOOTDELAY	5 */
#define CONFIG_BOOTCOMMAND \
		"dhcp;bootm"
#define CONFIG_BOOTARGS \
		"console=ttyS0,115200n8 " \
		"root=/dev/mmcblk0p1 rootwait rootfstype=ext3 ro"

#define CONFIG_CMDLINE_EDITING
#define CONFIG_VERSION_VARIABLE
#define CONFIG_TIMESTAMP

#define CONFIG_NET_RETRY_COUNT 10

/* U-Boot memory configuration */
#define CONFIG_STACKSIZE		SZ_256K		/* regular stack */
#define CONFIG_SYS_MALLOC_LEN		SZ_512K		/* malloc() arena */
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* for initial data */
#define CONFIG_SYS_MEMTEST_START	0x87000000	/* physical address */
#define CONFIG_SYS_MEMTEST_END		0x88000000	/* test 16MB RAM */

/* Linux interfacing */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SYS_BARGSIZE	1024			/* bootarg Size */
#define CONFIG_SYS_LOAD_ADDR	0x80700000		/* kernel address */


/* NAND configuration ... socketed with two chipselects.  It normally comes
 * with a 2GByte SLC part with 2KB pages (and 128KB erase blocks); other
 * 2GByte parts may have 4KB pages, 256KB erase blocks, and use MLC.  (MLC
 * pretty much demands the 4-bit ECC support.)  You can of course swap in
 * other parts, including small page ones.
 *
 * This presents a single read-only partition for all bootloader stuff.
 * UBL (1+ block), U-Boot (256KB+), U-Boot environment (one block), and
 * some extra space to help cope with bad blocks in that data.  Linux
 * shouldn't care about its detailed layout, and will probably want to use
 * UBI/UBFS for the rest (except maybe on smallpage chips).  It's easy to
 * override this default partitioning using MTDPARTS and cmdlinepart.
 */
#define MTDIDS_DEFAULT		"nand0=davinci_nand.0"

#ifdef CONFIG_SYS_NAND_LARGEPAGE
/*  Use same layout for 128K/256K blocks; allow some bad blocks */
#define PART_BOOT		"2m(bootloader)ro,"
#else
/* Assume 16K erase blocks; allow a few bad ones. */
#define PART_BOOT		"512k(bootloader)ro,"
#endif

#define PART_KERNEL		"4m(kernel),"	/* kernel + initramfs */
#define PART_REST		"-(filesystem)"

#define MTDPARTS_DEFAULT	\
	"mtdparts=davinci_nand.0:" PART_BOOT PART_KERNEL PART_REST

#endif /* __CONFIG_H */
