/*
 * Configuation settings for the Renesas SH7763RDP board
 *
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SH7763RDP_H
#define __SH7763RDP_H

#define CONFIG_SH		1
#define CONFIG_SH4		1
#define CONFIG_CPU_SH7763	1
#define CONFIG_SH7763RDP	1
#define __LITTLE_ENDIAN		1

/*
 * Command line configuration.
 */
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_NFS
#define CONFIG_CMD_JFFS2

#define CONFIG_BOOTDELAY        -1
#define CONFIG_BOOTARGS         "console=ttySC2,115200 root=1f01"
#define CONFIG_ENV_OVERWRITE    1

#define CONFIG_VERSION_VARIABLE
#undef  CONFIG_SHOW_BOOT_PROGRESS

/* SCIF */
#define CONFIG_SCIF_CONSOLE        1
#define CONFIG_BAUDRATE         115200
#define CONFIG_CONS_SCIF2		1

#define CONFIG_SYS_TEXT_BASE	0x8FFC0000
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		256	/* Buffer size for input from the Console */
#define CONFIG_SYS_PBSIZE		256	/* Buffer size for Console output */
#define CONFIG_SYS_MAXARGS		16	/* max args accepted for monitor commands */
#define CONFIG_SYS_BARGSIZE	512	/* Buffer size for Boot Arguments
								passed to kernel */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }	/* List of legal baudrate
												settings for this board */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		(0x8C000000)
#define CONFIG_SYS_SDRAM_SIZE		(64 * 1024 * 1024)
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (60 * 1024 * 1024))

/* Flash(NOR) */
#define CONFIG_SYS_FLASH_BASE		(0xA0000000)
#define CONFIG_SYS_FLASH_CFI_WIDTH (FLASH_CFI_16BIT)
#define CONFIG_SYS_MAX_FLASH_BANKS (1)
#define CONFIG_SYS_MAX_FLASH_SECT  (520)

/* U-boot setting */
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 4 * 1024 * 1024)
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_MONITOR_LEN		(128 * 1024)
/* Size of DRAM reserved for malloc() use */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#undef  CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */
/* Timeout for Flash erase operations (in ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(3 * 1000)
/* Timeout for Flash write operations (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(3 * 1000)
/* Timeout for Flash set sector lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_LOCK_TOUT		(3 * 1000)
/* Timeout for Flash clear lock bit operations (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	(3 * 1000)
/* Use hardware flash sectors protection instead of U-Boot software protection */
#undef  CONFIG_SYS_FLASH_PROTECTION
#undef  CONFIG_SYS_DIRECT_FLASH_TFTP
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + (1 * CONFIG_ENV_SECT_SIZE))
/* Offset of env Flash sector relative to CONFIG_SYS_FLASH_BASE */
#define CONFIG_ENV_OFFSET		(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_FLASH_BASE + (2 * CONFIG_ENV_SECT_SIZE))

/* Clock */
#define CONFIG_SYS_CLK_FREQ	66666666
#define CONFIG_SYS_TMU_CLK_DIV		(4)	/* 4 (default), 16, 64, 256 or 1024 */
#define CONFIG_SYS_HZ		1000

/* Ether */
#define CONFIG_SH_ETHER 1
#define CONFIG_SH_ETHER_USE_PORT (1)
#define CONFIG_SH_ETHER_PHY_ADDR (0x01)
#define CONFIG_PHYLIB
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

#endif /* __SH7763RDP_H */
