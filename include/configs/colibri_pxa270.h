/*
 * Toradex Colibri PXA270 configuration file
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
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

/*
 * High Level Board Configuration Options
 */
#define	CONFIG_CPU_PXA27X		1	/* Marvell PXA270 CPU */
#define	CONFIG_SYS_TEXT_BASE		0x0

/*
 * Environment settings
 */
#define	CONFIG_ENV_OVERWRITE
#define	CONFIG_SYS_MALLOC_LEN		(128 * 1024)
#define	CONFIG_ARCH_CPU_INIT
#define	CONFIG_BOOTCOMMAND						\
	"if mmc init && fatload mmc 0 0xa0000000 uImage; then "		\
		"bootm 0xa0000000; "					\
	"fi; "								\
	"if usb reset && fatload usb 0 0xa0000000 uImage; then "	\
		"bootm 0xa0000000; "					\
	"fi; "								\
	"bootm 0x80000;"
#define	CONFIG_BOOTARGS			"console=tty0 console=ttyS0,115200"
#define	CONFIG_TIMESTAMP
#define	CONFIG_BOOTDELAY		2	/* Autoboot delay */
#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS
#define	CONFIG_LZMA			/* LZMA compression support */
#define	CONFIG_OF_LIBFDT

/*
 * Serial Console Configuration
 */
#define	CONFIG_PXA_SERIAL
#define	CONFIG_FFUART			1
#define	CONFIG_BAUDRATE			115200

/*
 * Bootloader Components Configuration
 */
#include <config_cmd_default.h>

#define	CONFIG_CMD_NET
#define	CONFIG_CMD_ENV
#undef	CONFIG_CMD_IMLS
#define	CONFIG_CMD_MMC
#define	CONFIG_CMD_USB
#define	CONFIG_CMD_FLASH

/*
 * Networking Configuration
 *  chip on the Voipac PXA270 board
 */
#ifdef	CONFIG_CMD_NET
#define	CONFIG_CMD_PING
#define	CONFIG_CMD_DHCP

#define	CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x08000000
#define DM9000_IO			(CONFIG_DM9000_BASE)
#define DM9000_DATA			(CONFIG_DM9000_BASE + 4)
#define	CONFIG_NET_RETRY_COUNT		10

#define	CONFIG_BOOTP_BOOTFILESIZE
#define	CONFIG_BOOTP_BOOTPATH
#define	CONFIG_BOOTP_GATEWAY
#define	CONFIG_BOOTP_HOSTNAME
#endif

/*
 * HUSH Shell Configuration
 */
#define	CONFIG_SYS_HUSH_PARSER		1

#define	CONFIG_SYS_LONGHELP
#ifdef	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT		"$ "
#else
#define	CONFIG_SYS_PROMPT		"=> "
#endif
#define	CONFIG_SYS_CBSIZE		256
#define	CONFIG_SYS_PBSIZE		\
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define	CONFIG_SYS_MAXARGS		16
#define	CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define	CONFIG_SYS_DEVICE_NULLDEV	1
#define	CONFIG_CMDLINE_EDITING		1
#define	CONFIG_AUTO_COMPLETE		1


/*
 * Clock Configuration
 */
#define	CONFIG_SYS_HZ			1000		/* Timer @ 3250000 Hz */
#define	CONFIG_SYS_CPUSPEED		0x290		/* 520MHz */

/*
 * DRAM Map
 */
#define	CONFIG_NR_DRAM_BANKS		1		/* We have 1 bank of DRAM */
#define	PHYS_SDRAM_1			0xa0000000	/* SDRAM Bank #1 */
#define	PHYS_SDRAM_1_SIZE		0x04000000	/* 64 MB */

#define	CONFIG_SYS_DRAM_BASE		0xa0000000	/* CS0 */
#define	CONFIG_SYS_DRAM_SIZE		0x04000000	/* 64 MB DRAM */

#define CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM */

#define	CONFIG_SYS_LOAD_ADDR		PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		0x5c010000

/*
 * NOR FLASH
 */
#ifdef	CONFIG_CMD_FLASH
#define	PHYS_FLASH_1			0x00000000	/* Flash Bank #1 */
#define	CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define	CONFIG_SYS_FLASH_CFI
#define	CONFIG_FLASH_CFI_DRIVER		1

#define	CONFIG_SYS_MAX_FLASH_SECT	(4 + 255)
#define	CONFIG_SYS_MAX_FLASH_BANKS	1

#define	CONFIG_SYS_FLASH_ERASE_TOUT	(25 * CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_WRITE_TOUT	(25 * CONFIG_SYS_HZ)

#define	CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
#define	CONFIG_SYS_FLASH_PROTECTION		1

#define CONFIG_ENV_IS_IN_FLASH		1

#else	/* No flash */
#define	CONFIG_SYS_NO_FLASH
#define	CONFIG_SYS_ENV_IS_NOWHERE
#endif

#define	CONFIG_SYS_MONITOR_BASE		0x0
#define	CONFIG_SYS_MONITOR_LEN		0x80000

#define	CONFIG_ENV_ADDR			\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define	CONFIG_ENV_SIZE			0x40000
#define	CONFIG_ENV_SECT_SIZE		0x40000
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)

/*
 * GPIO settings
 */
#define	CONFIG_SYS_GPSR0_VAL	0x00000000
#define	CONFIG_SYS_GPSR1_VAL	0x00020000
#define	CONFIG_SYS_GPSR2_VAL	0x0002C000
#define	CONFIG_SYS_GPSR3_VAL	0x00000000

#define	CONFIG_SYS_GPCR0_VAL	0x00000000
#define	CONFIG_SYS_GPCR1_VAL	0x00000000
#define	CONFIG_SYS_GPCR2_VAL	0x00000000
#define	CONFIG_SYS_GPCR3_VAL	0x00000000

#define	CONFIG_SYS_GPDR0_VAL	0x08000000
#define	CONFIG_SYS_GPDR1_VAL	0x0002A981
#define	CONFIG_SYS_GPDR2_VAL	0x0202FC00
#define	CONFIG_SYS_GPDR3_VAL	0x00000000

#define	CONFIG_SYS_GAFR0_L_VAL	0x00100000
#define	CONFIG_SYS_GAFR0_U_VAL	0x00C00010
#define	CONFIG_SYS_GAFR1_L_VAL	0x999A901A
#define	CONFIG_SYS_GAFR1_U_VAL	0xAAA00008
#define	CONFIG_SYS_GAFR2_L_VAL	0xAAAAAAAA
#define	CONFIG_SYS_GAFR2_U_VAL	0x0109A000
#define	CONFIG_SYS_GAFR3_L_VAL	0x54000300
#define	CONFIG_SYS_GAFR3_U_VAL	0x00024001

#define	CONFIG_SYS_PSSR_VAL	0x30

/*
 * Clock settings
 */
#define	CONFIG_SYS_CKEN		0x00500240
#define	CONFIG_SYS_CCCR		0x02000290

/*
 * Memory settings
 */
#define	CONFIG_SYS_MSC0_VAL	0x000095f2
#define	CONFIG_SYS_MSC1_VAL	0x00007ff4
#define	CONFIG_SYS_MSC2_VAL	0x00000000
#define	CONFIG_SYS_MDCNFG_VAL	0x08000ac9
#define	CONFIG_SYS_MDREFR_VAL	0x2013e01e
#define	CONFIG_SYS_MDMRS_VAL	0x00320032
#define	CONFIG_SYS_FLYCNFG_VAL	0x00000000
#define	CONFIG_SYS_SXCNFG_VAL	0x40044004

/*
 * PCMCIA and CF Interfaces
 */
#define	CONFIG_SYS_MECR_VAL	0x00000001
#define	CONFIG_SYS_MCMEM0_VAL	0x00014307
#define	CONFIG_SYS_MCMEM1_VAL	0x00014307
#define	CONFIG_SYS_MCATT0_VAL	0x0001c787
#define	CONFIG_SYS_MCATT1_VAL	0x0001c787
#define	CONFIG_SYS_MCIO0_VAL	0x0001430f
#define	CONFIG_SYS_MCIO1_VAL	0x0001430f

#include "pxa-common.h"

#endif	/* __CONFIG_H */
