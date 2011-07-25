/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Integrator AP board.
 *.
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
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_HZ_CLOCK		24000000	/* Timer 1 is clocked at 24Mhz */
#define CONFIG_SYS_TIMERBASE		0x13000100	/* Timer1		       */

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r during start up */

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_CM_INIT		1
#define CONFIG_CM_REMAP		1
#define CONFIG_CM_SPD_DETECT

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)

/*
 * PL010 Configuration
 */
#define CONFIG_PL010_SERIAL
#define CONFIG_CONS_INDEX	0
#define CONFIG_BAUDRATE		38400
#define CONFIG_PL01x_PORTS	{ (void *) (CONFIG_SYS_SERIAL0), (void *) (CONFIG_SYS_SERIAL1) }
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_SERIAL0		0x16000000
#define CONFIG_SYS_SERIAL1		0x17000000

/*#define CONFIG_NET_MULTI */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */

#define CONFIG_CMD_IMI
#define CONFIG_CMD_BDI
#define CONFIG_CMD_MEMORY


#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS		"root=/dev/mtdblock0 mem=32M console=ttyAM0 console=tty"
#define CONFIG_BOOTCOMMAND	""

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory	    */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT	"Integrator-AP # "	/* Monitor Command Prompt   */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"# "
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_LOAD_ADDR	0x7fc0	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000	/* 32 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE PHYS_SDRAM_1_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET (CONFIG_SYS_SDRAM_BASE + \
				    CONFIG_SYS_INIT_RAM_SIZE - \
				    GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_FLASH_BASE	0x24000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of memory banks */
/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Write */
#define CONFIG_SYS_MAX_FLASH_SECT	128
#define CONFIG_ENV_SIZE			32768


/*-----------------------------------------------------------------------
 * PCI definitions
 */

#ifdef CONFIG_PCI			/* pci support	*/
#undef CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW	1	/* show pci devices on startup	*/
#define DEBUG

#define CONFIG_EEPRO100
#define CONFIG_SYS_RX_ETH_BUFFER	8	/* use 8 rx buffer on eepro100	*/

#define INTEGRATOR_BOOT_ROM_BASE	0x20000000
#define INTEGRATOR_HDR0_SDRAM_BASE	0x80000000

/* PCI Base area */
#define INTEGRATOR_PCI_BASE		0x40000000
#define INTEGRATOR_PCI_SIZE		0x3FFFFFFF

/* memory map as seen by the CPU on the local bus */
#define CPU_PCI_IO_ADRS		0x60000000	/* PCI I/O space base */
#define CPU_PCI_IO_SIZE		0x10000

#define CPU_PCI_CNFG_ADRS	0x61000000	/* PCI config space */
#define CPU_PCI_CNFG_SIZE	0x1000000

#define PCI_MEM_BASE		0x40000000   /* 512M to xxx */
/*  unused 256M from A0000000-AFFFFFFF might be used for I2O ??? */
#define INTEGRATOR_PCI_IO_BASE	0x60000000   /* 16M to xxx */
/*  unused (128-16)M from B1000000-B7FFFFFF */
#define PCI_CONFIG_BASE		0x61000000   /* 16M to xxx */
/*  unused ((128-16)M - 64K) from XXX */

#define PCI_V3_BASE		0x62000000

/* V3 PCI bridge controller */
#define V3_BASE			0x62000000    /* V360EPC registers */

#define PCI_ENET0_IOADDR	(CPU_PCI_IO_ADRS)
#define PCI_ENET0_MEMADDR	(PCI_MEM_BASE)


#define V3_PCI_VENDOR		0x00000000
#define V3_PCI_DEVICE		0x00000002
#define V3_PCI_CMD		0x00000004
#define V3_PCI_STAT		0x00000006
#define V3_PCI_CC_REV		0x00000008
#define V3_PCI_HDR_CF		0x0000000C
#define V3_PCI_IO_BASE		0x00000010
#define V3_PCI_BASE0		0x00000014
#define V3_PCI_BASE1		0x00000018
#define V3_PCI_SUB_VENDOR	0x0000002C
#define V3_PCI_SUB_ID		0x0000002E
#define V3_PCI_ROM		0x00000030
#define V3_PCI_BPARAM		0x0000003C
#define V3_PCI_MAP0		0x00000040
#define V3_PCI_MAP1		0x00000044
#define V3_PCI_INT_STAT		0x00000048
#define V3_PCI_INT_CFG		0x0000004C
#define V3_LB_BASE0		0x00000054
#define V3_LB_BASE1		0x00000058
#define V3_LB_MAP0		0x0000005E
#define V3_LB_MAP1		0x00000062
#define V3_LB_BASE2		0x00000064
#define V3_LB_MAP2		0x00000066
#define V3_LB_SIZE		0x00000068
#define V3_LB_IO_BASE		0x0000006E
#define V3_FIFO_CFG		0x00000070
#define V3_FIFO_PRIORITY	0x00000072
#define V3_FIFO_STAT		0x00000074
#define V3_LB_ISTAT		0x00000076
#define V3_LB_IMASK		0x00000077
#define V3_SYSTEM		0x00000078
#define V3_LB_CFG		0x0000007A
#define V3_PCI_CFG		0x0000007C
#define V3_DMA_PCI_ADR0		0x00000080
#define V3_DMA_PCI_ADR1		0x00000090
#define V3_DMA_LOCAL_ADR0	0x00000084
#define V3_DMA_LOCAL_ADR1	0x00000094
#define V3_DMA_LENGTH0		0x00000088
#define V3_DMA_LENGTH1		0x00000098
#define V3_DMA_CSR0		0x0000008B
#define V3_DMA_CSR1		0x0000009B
#define V3_DMA_CTLB_ADR0	0x0000008C
#define V3_DMA_CTLB_ADR1	0x0000009C
#define V3_DMA_DELAY		0x000000E0
#define V3_MAIL_DATA		0x000000C0
#define V3_PCI_MAIL_IEWR	0x000000D0
#define V3_PCI_MAIL_IERD	0x000000D2
#define V3_LB_MAIL_IEWR		0x000000D4
#define V3_LB_MAIL_IERD		0x000000D6
#define V3_MAIL_WR_STAT		0x000000D8
#define V3_MAIL_RD_STAT		0x000000DA
#define V3_QBA_MAP		0x000000DC

/* SYSTEM register bits */
#define V3_SYSTEM_M_RST_OUT		(1 << 15)
#define V3_SYSTEM_M_LOCK		(1 << 14)

/*  PCI_CFG bits */
#define V3_PCI_CFG_M_RETRY_EN		(1 << 10)
#define V3_PCI_CFG_M_AD_LOW1		(1 << 9)
#define V3_PCI_CFG_M_AD_LOW0		(1 << 8)

/* PCI MAP register bits (PCI -> Local bus) */
#define V3_PCI_MAP_M_MAP_ADR		0xFFF00000
#define V3_PCI_MAP_M_RD_POST_INH	(1 << 15)
#define V3_PCI_MAP_M_ROM_SIZE		(1 << 11 | 1 << 10)
#define V3_PCI_MAP_M_SWAP		(1 << 9 | 1 << 8)
#define V3_PCI_MAP_M_ADR_SIZE		0x000000F0
#define V3_PCI_MAP_M_REG_EN		(1 << 1)
#define V3_PCI_MAP_M_ENABLE		(1 << 0)

/* 9 => 512M window size */
#define V3_PCI_MAP_M_ADR_SIZE_512M	0x00000090

/* A => 1024M window size */
#define V3_PCI_MAP_M_ADR_SIZE_1024M	0x000000A0

/* LB_BASE register bits (Local bus -> PCI) */
#define V3_LB_BASE_M_MAP_ADR		0xFFF00000
#define V3_LB_BASE_M_SWAP		(1 << 8 | 1 << 9)
#define V3_LB_BASE_M_ADR_SIZE		0x000000F0
#define V3_LB_BASE_M_PREFETCH		(1 << 3)
#define V3_LB_BASE_M_ENABLE		(1 << 0)

/* PCI COMMAND REGISTER bits */
#define V3_COMMAND_M_FBB_EN		(1 << 9)
#define V3_COMMAND_M_SERR_EN		(1 << 8)
#define V3_COMMAND_M_PAR_EN		(1 << 6)
#define V3_COMMAND_M_MASTER_EN		(1 << 2)
#define V3_COMMAND_M_MEM_EN		(1 << 1)
#define V3_COMMAND_M_IO_EN		(1 << 0)

#define INTEGRATOR_SC_BASE		0x11000000
#define INTEGRATOR_SC_PCIENABLE_OFFSET	0x18
#define INTEGRATOR_SC_PCIENABLE \
			(INTEGRATOR_SC_BASE + INTEGRATOR_SC_PCIENABLE_OFFSET)

#endif /* CONFIG_PCI */
/*-----------------------------------------------------------------------
 * There are various dependencies on the core module (CM) fitted
 * Users should refer to their CM user guide
 * - when porting adjust u-boot/Makefile accordingly
 *   to define the necessary CONFIG_ s for the CM involved
 * see e.g. integratorcp_CM926EJ-S_config
 */
#include "armcoremodule.h"

#endif	/* __CONFIG_H */
