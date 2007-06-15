/*
 * (C) Copyright 2002
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
 * board/config_CPCI440.h - configuration for esd CPCI-440 board
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_CPCI440		1	    /* Board is ebony		*/
#define CONFIG_440GP		1	    /* Specifc GP support	*/
#define CONFIG_440		1	    /* ... PPC440 family	*/
#define CONFIG_4xx		1	    /* ... PPC4xx family	*/
#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_early_init_f	*/
#undef	CFG_DRAM_TEST			    /* Disable-takes long time! */
#define CONFIG_SYS_CLK_FREQ	33330000 /* external frequency to pll	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE	    0x00000000	    /* _must_ be 0		*/
#define CFG_FLASH_BASE	    0xff800000	    /* start of FLASH		*/
#if 1
#define CFG_MONITOR_BASE    0xfffc0000	    /* start of monitor		*/
#else
#define CFG_MONITOR_BASE    0x01fc0000	    /* start of monitor		*/
#endif
#define CFG_PERIPHERAL_BASE 0xe0000000	    /* internal peripherals	*/
#define CFG_ISRAM_BASE	    0xc0000000	    /* internal SRAM		*/

#define CFG_FPGA_BASE	    (CFG_PERIPHERAL_BASE + 0x08300000)
#define CFG_NVRAM_BASE_ADDR (CFG_PERIPHERAL_BASE + 0x08000000)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_ADDR   CFG_ISRAM_BASE  /* Initial RAM address	*/
#define CFG_INIT_RAM_END    0x2000	    /* End of used area in RAM	*/
#define CFG_GBL_DATA_SIZE  128		    /* num bytes initial data	*/

#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN	    (192 * 1024)    /* Reserve 192 kB for Mon	*/
#define CFG_MALLOC_LEN	    (128 * 1024)    /* Reserve 128 kB for malloc*/

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#undef CFG_EXT_SERIAL_CLOCK /*	 (1843200 * 6)	 / * Ext clk @ 11.059 MHz */
#define CONFIG_BAUDRATE		9600

#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400}

/*-----------------------------------------------------------------------
 * NVRAM/RTC
 *
 * NOTE: Upper 8 bytes of NVRAM is where the RTC registers are located.
 * The DS1743 code assumes this condition (i.e. -- it assumes the base
 * address for the RTC registers is:
 *
 *	CFG_NVRAM_BASE_ADDR + CFG_NVRAM_SIZE
 *
 *----------------------------------------------------------------------*/
#define CFG_NVRAM_SIZE	    (0x2000 - 8)    /* NVRAM size(8k)- RTC regs */
#define CONFIG_RTC_DS174x	1		    /* DS1743 RTC		*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#if 1 /* test-only */

#define CFG_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_FLASH_INCREMENT	0	/* there is only one bank		*/
#define CFG_FLASH_PROTECTION	1	/* use hardware protection		*/
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#undef CFG_FLASH_BASE
#define CFG_FLASH_BASE		0xFF800000 /* test-only...*/

#else /* test-only */

#define CFG_MAX_FLASH_BANKS	3		    /* number of banks	    */
#define CFG_MAX_FLASH_SECT	32		    /* sectors per device   */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#endif

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#if 0 /* test-only */
#define CFG_ENV_IS_IN_NVRAM	1	    /* Environment uses NVRAM	*/
#undef	CFG_ENV_IS_IN_FLASH		    /* ... not in flash		*/
#undef	CFG_ENV_IS_IN_EEPROM		    /* ... not in EEPROM	*/

#define CFG_ENV_SIZE		0x1000	    /* Size of Environment vars */
#define CFG_ENV_ADDR		\
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_ENV_SIZE)
#else

#if 0 /* test-only */
#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x010	/* environment starts at the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x800	/* 2048 bytes may be used for env vars*/
				   /* total size of a CAT24WC16 is 2048 bytes */
#else
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/
#endif

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC16) for environment
 */
#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08		*/
#define CFG_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CFG_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has	*/
					/* 16 byte page write mode using*/
					/* last 4 bits of the address	*/
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */
#define CFG_EEPROM_PAGE_WRITE_ENABLE

#endif

#undef	CONFIG_BOOTARGS
#undef	CONFIG_BOOTCOMMAND

#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/
#define CONFIG_BAUDRATE		9600

#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1	/* PHY address			*/
#define CONFIG_LXT971_NO_SLEEP  1       /* disable sleep mode in LXT971 */

#if 0 /* test-only */
#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_I2C	| \
				CFG_CMD_KGDB	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_DATE	| \
				CFG_CMD_BEDBUG	| \
				CFG_CMD_ELF	)
#else
#define CONFIG_COMMANDS	      ( CONFIG_CMD_DFL	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_ELF	| \
				CFG_CMD_DATE	| \
				CFG_CMD_I2C	| \
				CFG_CMD_EEPROM	)
/* test-only: support fehlt bisher... */
/*				CFG_CMD_IDE	| \*/
/*				CFG_CMD_PCI	| \*/
#endif

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

#undef CONFIG_SPD_EEPROM       /* don't use SPD EEPROM for setup    */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#if 0 /* test-only */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#endif


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#if 0
#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW            /* print pci devices @ startup  */

#define CONFIG_PCI_BOOTDELAY    0       /* enable pci bootdelay variable*/

#define CFG_PCI_SUBSYS_VENDORID 0x0000	/* PCI Vendor ID: to-do!!!	*/
#define CFG_PCI_SUBSYS_DEVICEID 0x0000	/* PCI Device ID: to-do!!!	*/
#define CFG_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CFG_PCI_PTM1MS	0x80000001	/* 2GB, enable hard-wired to 1	*/
#define CFG_PCI_PTM1PCI 0x00000000	/* Host: use this pci address	*/
#define CFG_PCI_PTM2LA	0x00000000	/* disabled			*/
#define CFG_PCI_PTM2MS	0x00000000	/* disabled			*/
#define CFG_PCI_PTM2PCI 0x04000000	/* Host: use this pci address	*/
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		32768	/* For AMCC 440 CPUs			*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif


/* Configuration Port location */
#define CONFIG_PORT_ADDR	0xF0000500

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS	0x50

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif
#endif	/* __CONFIG_H */
