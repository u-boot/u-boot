/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Compact Integrator board.
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

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CFG_MEMTEST_START	0x100000
#define CFG_MEMTEST_END		0x10000000
#define CFG_HZ			1000
#define CFG_HZ_CLOCK		1000000	/* Timer 1 is clocked at 1Mhz */
#define CFG_TIMERBASE		0x13000100

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R		1	/* call misc_init_r during start up */
/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_SMC91111_BASE    0xC8000000
#undef CONFIG_SMC91111_EXT_PHY

/*
 * NS16550 Configuration
 */
#define CFG_PL011_SERIAL
#define CONFIG_PL011_CLOCK	14745600
#define CONFIG_PL01x_PORTS	{ (void *)CFG_SERIAL0, (void *)CFG_SERIAL1 }
#define CONFIG_CONS_INDEX	0
#define CONFIG_BAUDRATE		38400
#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }
#define CFG_SERIAL0		0x16000000
#define CFG_SERIAL1		0x17000000

/*
#define CONFIG_COMMANDS		(CFG_CMD_DFL | CFG_CMD_PCI)
*/
#define CONFIG_COMMANDS		(CFG_CMD_DHCP | CFG_CMD_IMI | CFG_CMD_NET | CFG_CMD_PING | \
				 CFG_CMD_BDI | CFG_CMD_MEMORY | CFG_CMD_FLASH | CFG_CMD_ENV \
				)

/* #define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT */

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#if 0
#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS	"root=/dev/nfs mem=128M ip=dhcp netdev=27,0,0xfc800000,0xfc800010,eth0"
#define CONFIG_BOOTCOMMAND "bootp ; bootm"
#endif

/* Flash loaded
   - U-Boot	     
   - u-linux
   - system.cramfs
*/
#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS	"root=/dev/mtdblock2 mem=128M ip=dhcp netdev=27,0, \
0xfc800000,0xfc800010,eth0 video=clcdfb:0"
#define CONFIG_BOOTCOMMAND "cp 0x24040000 0x7fc0 0x80000; bootm"

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory     */
#define CFG_PROMPT	"Integrator-CP # "	/* Monitor Command Prompt   */
#define CFG_CBSIZE	256			/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CFG_PBSIZE	(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS	16			/* max number of command args   */
#define CFG_BARGSIZE	CFG_CBSIZE		/* Boot Argument Buffer Size    */

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CFG_LOAD_ADDR	0x7fc0	/* default load address */

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
#define CONFIG_NR_DRAM_BANKS    1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1            0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE       0x08000000	/* 128 MB */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_FLASH_BASE          0x24000000
#define CFG_MAX_FLASH_SECT 	64
#define CFG_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define PHYS_FLASH_SIZE         0x01000000	/* 16MB */
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Write */

#define CFG_MONITOR_BASE	0x24F40000
#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_ADDR		0x24F00000
#define CFG_ENV_SECT_SIZE	0x40000		/* 256KB */
#define CFG_ENV_SIZE		8192		/* 8KB */

/*-----------------------------------------------------------------------
 * There are various dependencies on the core module (CM) fitted
 * Users should refer to their CM user guide
 * - when porting adjust u-boot/Makefile accordingly
 *   to define the necessary CONFIG_ s for the CM involved
 * see e.g. integratorcp_CM926EJ-S_config
 */

#define CM_BASE		0x10000000

/* CM registers common to all integrator/CP CMs */
#define OS_CTRL			0x0000000C
#define CMMASK_REMAP		0x00000005	/* set remap & led           */
#define CMMASK_RESET		0x00000008
#define OS_LOCK	        	0x00000014
#define CMVAL_LOCK	     	0x0000A000	/* locking value             */
#define CMMASK_LOCK		0x0000005F	/* locking value             */
#define CMVAL_UNLOCK		0x00000000	/* any value != CM_LOCKVAL   */
#define OS_SDRAM		0x00000020
#define OS_INIT			0x00000024
#define CMMASK_MAP_SIMPLE	0xFFFDFFFF	/* simple mapping */
#define CMMASK_TCRAM_DISABLE	0xFFFEFFFF	/* TCRAM disabled */
#define CMMASK_LOWVEC		0x00000004	/* vectors @ 0x00000000 */
#if defined (CONFIG_CM10200E) || defined (CONFIG_CM10220E)
#define CMMASK_INIT_102		0x00000300	/* see CM102xx ref manual 
						 * - PLL test clock bypassed
						 * - bus clock ratio 2
						 * - little endian
						 * - vectors at zero
						 */
#endif /* CM1022xx */ 

#define CMMASK_LE		0x00000008	/* little endian */
#define CMMASK_CMxx6_COMMON	0x00000100      /* Common value for CMxx6  
						 * - divisor/ratio b00000001
						 *                 bx
						 * - HCLKDIV       b000
						 *                 bxx
						 * - PLL BYPASS    b00
						 */

/* Determine CM characteristics */

#undef	CONFIG_CM_MULTIPLE_SSRAM
#undef	CONFIG_CM_SPD_DETECT	
#undef	CONFIG_CM_REMAP		
#undef	CONFIG_CM_INIT	
#undef	CONFIG_CM_TCRAM	  

#if defined (CONFIG_CM946E_S) || defined (CONFIG_CM966E_S)
#define	CONFIG_CM_MULTIPLE_SSRAM	/* CM has multiple SSRAM mapping */
#endif

#ifndef	CONFIG_CM922t_XA10					
#define CONFIG_CM_SPD_DETECT			/* CM supports SPD query      */
#define OS_SPD			0x00000100	/* Address of SPD data        */
#define CONFIG_CM_REMAP				/* CM supports remapping      */
#define CONFIG_CM_INIT				/* CM has initialization reg  */
#endif	

#if defined(CONFIG_CM926EJ_S)   || defined (CONFIG_CM946E_S)	|| \
    defined(CONFIG_CM966E_S)    || defined (CONFIG_CM1026EJ_S)	|| \
    defined(CONFIG_CM1136JF_S)
#define CONFIG_CM_TCRAM				/* CM has TCRAM  */
#endif				

#endif /* __CONFIG_H */
