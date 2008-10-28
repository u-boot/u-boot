/*
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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

/************************************************************************
 * config for XPedite1000 from XES Inc.
 * Ported from EBONY config by Travis B. Sawyer <tsawyer@sandburst.com>
 * (C) Copyright 2003 Sandburst Corporation
 * board/config_EBONY.h - configuration for AMCC 440GP Ref (Ebony)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_XPEDITE1K	1		/* Board is XPedite 1000 */
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_440		1
#define CONFIG_440GX		1		/* 440 GX */
#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_pre_init	*/
#undef	CONFIG_SYS_DRAM_TEST				/* Disable-takes long time! */
#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll */


/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_RTC	   | \
				 CONFIG_SYS_POST_I2C)

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_SDRAM_BASE	    0x00000000		/* _must_ be 0		*/
#define CONFIG_SYS_FLASH_BASE	    0xfff80000		/* start of FLASH	*/

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_FLASH_BASE	/* start of monitor	*/
#define CONFIG_SYS_PCI_MEMBASE	    0x80000000		/* mapped pci memory	*/
#define CONFIG_SYS_PERIPHERAL_BASE 0xe0000000		/* internal peripherals */
#define CONFIG_SYS_ISRAM_BASE	    0xc0000000		/* internal SRAM	*/
#define CONFIG_SYS_PCI_BASE	    0xd0000000		/* internal PCI regs	*/

#define CONFIG_SYS_NVRAM_BASE_ADDR (CONFIG_SYS_PERIPHERAL_BASE + 0x08000000)
#define CONFIG_SYS_GPIO_BASE	    (CONFIG_SYS_PERIPHERAL_BASE + 0x00000700)

#define USR_LED0	    0x00000080
#define USR_LED1	    0x00000100
#define USR_LED2	    0x00000200
#define USR_LED3	    0x00000400

#ifndef __ASSEMBLY__
extern unsigned long in32(unsigned int);
extern void out32(unsigned int, unsigned long);

#define LED0_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED0))
#define LED1_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED1))
#define LED2_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED2))
#define LED3_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED3))

#define LED0_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED0))
#define LED1_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED1))
#define LED2_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED2))
#define LED3_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED3))
#endif

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_TEMP_STACK_OCM  1
#define CONFIG_SYS_OCM_DATA_ADDR   CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_ADDR   CONFIG_SYS_ISRAM_BASE  /* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_END    0x2000	    /* End of used area in RAM	*/
#define CONFIG_SYS_GBL_DATA_SIZE   128		    /* num bytes initial data	*/


#define CONFIG_SYS_GBL_DATA_OFFSET (CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_POST_WORD_ADDR  (CONFIG_SYS_GBL_DATA_OFFSET - 0x4)
#define CONFIG_SYS_INIT_SP_OFFSET  CONFIG_SYS_POST_WORD_ADDR

#define CONFIG_SYS_MONITOR_LEN	    (256 * 1024)    /* Reserve 256 kB for Mon	*/
#define CONFIG_SYS_MALLOC_LEN	    (128 * 1024)    /* Reserve 128 kB for malloc*/

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_BAUDRATE		9600

#define CONFIG_SYS_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400}

/*-----------------------------------------------------------------------
 * NVRAM/RTC
 *
 * NOTE: Upper 8 bytes of NVRAM is where the RTC registers are located.
 * The DS1743 code assumes this condition (i.e. -- it assumes the base
 * address for the RTC registers is:
 *
 *	CONFIG_SYS_NVRAM_BASE_ADDR + CONFIG_SYS_NVRAM_SIZE
 *
 *----------------------------------------------------------------------*/
/* TBS:	 Xpedite 1000 has STMicro M41T00 via IIC */
#define CONFIG_RTC_M41T11 1
#define CONFIG_SYS_I2C_RTC_ADDR 0x68
#define CONFIG_SYS_M41T11_BASE_YEAR 2000

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_FLASH_BANKS	1		    /* number of banks	    */
#define CONFIG_SYS_MAX_FLASH_SECT	8		    /* sectors per device   */

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	    /* Timeout for Flash Write (in ms)	*/

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS {0x54}	/* SPD i2c spd addresses	*/
#define CONFIG_VERY_BIG_RAM 1
/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	    /* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			    /* I2C bit-banged		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7f
#define CONFIG_SYS_I2C_NOPROBES	{0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x69}	/* Don't probe these addrs */

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_EEPROM 1
#define CONFIG_ENV_SIZE		0x100	    /* Size of Environment vars */
#define CONFIG_ENV_OFFSET		0x100
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50		/* this is actually the second page of the eeprom */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

#define CONFIG_BOOTARGS		"root=/dev/hda1 "
#define CONFIG_BOOTCOMMAND	"bootm ffc00000"    /* autoboot command */
#define CONFIG_BOOTDELAY	5		    /* disable autoboot */
#define CONFIG_BAUDRATE		9600

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII			1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address phy0 not populated */
#define CONFIG_PHY1_ADDR	1	/* PHY address phy1 not populated */
#define CONFIG_PHY2_ADDR	4	/* PHY address phy2 */
#define CONFIG_PHY3_ADDR	8	/* PHY address phy3 */
#define CONFIG_NET_MULTI	1
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_PHY_RESET        1       /* reset phy upon startup         */
#define CONFIG_SYS_RX_ETH_BUFFER   32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_HAS_ETH2		1	/* add support for "eth2addr"	*/
#define CONFIG_HAS_ETH3		1	/* add support for "eth3addr"	*/


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
#include <config_cmd_default.h>

#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_I2C
#define CONFIG_CMD_DATE
#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_PING
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MII
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_FAT


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI				    /* include pci support		*/
#define CONFIG_PCI_PNP				/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup	*/
#define CONFIG_SYS_PCI_TARGBASE    0x80000000	/* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT		    /* let board init pci target    */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1014	/* IBM */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */
#define CONFIG_SYS_PCI_FORCE_PCI_CONV          /* Force PCI Conventional Mode */
/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif
#endif	/* __CONFIG_H */
