/*
 * (C) Copyright 2005
 * STMicroelectronics.
 * Configuration settings for the STn8815 nomadik board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <nomadik.h>

#define CONFIG_ARM926EJS
#define CONFIG_NOMADIK
#define CONFIG_NOMADIK_8815
#define CONFIG_NOMADIK_NDK15
#define CONFIG_NOMADIK_NHK15

#define CONFIG_SKIP_LOWLEVEL_INIT /* we have already been loaded to RAM */

/* commands */
#include <config_cmd_default.h>

#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
/* There is no NOR flash, so undefine these commands */
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_SYS_NO_FLASH
/* There is NAND storage */
#define CONFIG_NAND_NOMADIK
#define CONFIG_CMD_JFFS2

/* user interface */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"Nomadik> "
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Arg Buffer Size */
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_LOAD_ADDR	0x800000	/* default load address */
#define CONFIG_SYS_LOADS_BAUD_CHANGE

/* boot config */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG
#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTARGS	"root=/dev/ram0 console=ttyAMA1,115200n8 init=linuxrc"
#define CONFIG_BOOTCOMMAND	"fsload 0x100000 kernel.uimg;" \
				" fsload 0x800000 initrd.gz.uimg;" \
				" bootm 0x100000 0x800000"

/* memory-related information */
#define CONFIG_NR_DRAM_BANKS	2
#define PHYS_SDRAM_1		0x00000000	/* DDR-SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000	/* 64 MB */
#define PHYS_SDRAM_2		0x08000000	/* SDR-SDRAM BANK #2*/
#define PHYS_SDRAM_2_SIZE	0x04000000	/* 64 MB */

#define CONFIG_STACKSIZE	(128 * 1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#  define CONFIG_STACKSIZE_IRQ	(4 * 1024)	/* IRQ stack */
#  define CONFIG_STACKSIZE_FIQ	(4 * 1024)	/* FIQ stack */
#endif

#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END		0x0FFFFFFF
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 256 * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* for initial data */

#define CONFIG_MISC_INIT_R	/* call misc_init_r during start up */

/* timing informazion */
#define CONFIG_SYS_HZ		(2400000 / 256)	/* Timer0: 2.4Mhz + divider */
#define CONFIG_SYS_TIMERBASE	0x101E2000

/* serial port (PL011) configuration */
#define CONFIG_PL011_SERIAL
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CFG_SERIAL0		0x101FD000
#define CFG_SERIAL1		0x101FB000

#define CONFIG_PL01x_PORTS	{ (void *)CFG_SERIAL0, (void *)CFG_SERIAL1 }
#define CONFIG_PL011_CLOCK	48000000

/* Ethernet */
#define PCI_MEMORY_VADDR	0xe8000000
#define PCI_IO_VADDR		0xee000000
#define __io(a)			((void __iomem *)(PCI_IO_VADDR + (a)))
#define __mem_isa(a)		((a) + PCI_MEMORY_VADDR)

#define CONFIG_DRIVER_SMC91111	/* Using SMC91c111*/
#define CONFIG_SMC91111_BASE	0x34000300
#undef  CONFIG_SMC91111_EXT_PHY	/* Internal PHY */
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_BOOTFILE		"uImage"

/* flash memory and filesystem information */
#define CONFIG_DOS_PARTITION
#define CONFIG_MTD_ONENAND_VERIFY_WRITE
#define CONFIG_SYS_ONENAND_BASE		0x30000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000 /* SMPS0n */

#ifdef CONFIG_BOOT_ONENAND

#   define CONFIG_CMD_ONENAND /* Temporary: nand and onenand can't coexist */
   /* Partition				Size	Start
    * XloaderTOC + X-Loader		256KB	0x00000000
    * Memory init function		256KB	0x00040000
    * U-Boot				2MB	0x00080000
    * Sysimage (kernel + ramdisk)	4MB	0x00280000
    * JFFS2 Root filesystem		22MB	0x00680000
    * JFFS2 User Data			227.5MB	0x01C80000
    */
#   define CONFIG_JFFS2_PART_SIZE	0x400000
#   define CONFIG_JFFS2_PART_OFFSET	0x280000

#   define CONFIG_ENV_IS_IN_ONENAND
#   define CONFIG_ENV_SIZE		(256 * 1024)
#   define CONFIG_ENV_ADDR		0x30300000

#else /* ! CONFIG_BOOT_ONENAND */

#   define CONFIG_CMD_NAND /* Temporary: nand and onenand can't coexist */

#   define CONFIG_JFFS2_DEV		"nand0"
#   define CONFIG_JFFS2_NAND		1 /* For the jffs2 support*/
#   define CONFIG_JFFS2_PART_SIZE	0x00300000
#   define CONFIG_JFFS2_PART_OFFSET	0x00280000

#   define CONFIG_ENV_IS_IN_NAND
#   define CONFIG_ENV_SIZE		0x20000 /* 128 Kb - one sector */
#   define CONFIG_ENV_OFFSET		(0x8000000 - CONFIG_ENV_SIZE)

#endif /* CONFIG_BOOT_ONENAND */

/* this is needed to make hello_world.c and other stuff happy */
#define CONFIG_SYS_MAX_FLASH_SECT	512
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#endif /* __CONFIG_H */
