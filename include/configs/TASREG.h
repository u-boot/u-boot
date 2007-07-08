/*
 * Configuation settings for the esd TASREG board.
 *
 * (C) Copyright 2004
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _TASREG_H
#define _TASREG_H

#ifndef __ASSEMBLY__
#include <asm/m5249.h>
#endif

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCF52x2			/* define processor family */
#define CONFIG_M5249			/* define processor type */

#define CONFIG_MISC_INIT_R      1       /* call misc_init_r()           */

#define CONFIG_BAUDRATE		19200
#define CFG_BAUDRATE_TABLE { 9600 , 19200 , 38400 , 57600, 115200 }

#undef  CONFIG_WATCHDOG

#undef CONFIG_MONITOR_IS_IN_RAM	              /* no pre-loader required!!! ;-) */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BSP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C

#undef CONFIG_CMD_NET


#define CONFIG_BOOTDELAY	3

#define CFG_PROMPT		"=> "
#define CFG_LONGHELP				/* undef to save memory		*/

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_DEVICE_NULLDEV	1	/* include nulldev device	*/
#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/
#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */

#define CFG_LOAD_ADDR	        0x200000	/* default load address */

#define CFG_MEMTEST_START	0x400
#define CFG_MEMTEST_END		0x380000

#define CFG_HZ			1000

/*
 * Clock configuration: enable only one of the following options
 */

#if 0 /* this setting will run the cpu at 11MHz */
#define CFG_PLL_BYPASS          1                /* bypass PLL for test purpose */
#undef  CFG_FAST_CLK                             /* MCF5249 can run at 140MHz   */
#define CFG_CLK		        11289600         /* PLL bypass                  */
#endif

#if 0 /* this setting will run the cpu at 70MHz */
#undef  CFG_PLL_BYPASS                           /* bypass PLL for test purpose */
#undef  CFG_FAST_CLK                             /* MCF5249 can run at 140MHz   */
#define CFG_CLK		        72185018         /* The next lower speed        */
#endif

#if 1 /* this setting will run the cpu at 140MHz */
#undef  CFG_PLL_BYPASS                           /* bypass PLL for test purpose */
#define CFG_FAST_CLK            1                /* MCF5249 can run at 140MHz   */
#define	CFG_CLK		        132025600        /* MCF5249 can run at 140MHz   */
#endif

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_MBAR		0x10000000	/* Register Base Addrs */
#define	CFG_MBAR2	        0x80000000

/*-----------------------------------------------------------------------
 * I2C
 */
#define	CONFIG_SOFT_I2C
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC32		*/
#define CFG_I2C_EEPROM_ADDR_LEN 2	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x01
#define CFG_EEPROM_PAGE_WRITE_BITS 5	/* The Catalyst CAT24WC32 has	*/
					/* 32 byte page write mode using*/
					/* last 5 bits of the address	*/
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */
#define CFG_EEPROM_PAGE_WRITE_ENABLE

#if defined (CONFIG_SOFT_I2C)
#if 0 /* push-pull */
#define	SDA	        0x00800000
#define	SCL	        0x00000008
#define DIR0            *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO_EN))
#define DIR1            *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO1_EN))
#define OUT0	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO_OUT))
#define OUT1	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO1_OUT))
#define IN0	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO_READ))
#define IN1	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO1_READ))
#define	I2C_INIT	{OUT1|=SDA;OUT0|=SCL;}
#define	I2C_READ	((IN1&SDA)?1:0)
#define	I2C_SDA(x)	{if(x)OUT1|=SDA;else OUT1&=~SDA;}
#define	I2C_SCL(x)	{if(x)OUT0|=SCL;else OUT0&=~SCL;}
#define	I2C_DELAY	{udelay(5);}
#define	I2C_ACTIVE	{DIR1|=SDA;}
#define	I2C_TRISTATE    {DIR1&=~SDA;}
#else /* open-collector */
#define	SDA	        0x00800000
#define	SCL	        0x00000008
#define DIR0            *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO_EN))
#define DIR1            *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO1_EN))
#define OUT0	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO_OUT))
#define OUT1	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO1_OUT))
#define IN0	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO_READ))
#define IN1	        *((volatile ulong*)(CFG_MBAR2+MCFSIM_GPIO1_READ))
#define	I2C_INIT	{DIR1&=~SDA;DIR0&=~SCL;OUT1&=~SDA;OUT0&=~SCL;}
#define	I2C_READ	((IN1&SDA)?1:0)
#define	I2C_SDA(x)	{if(x)DIR1&=~SDA;else DIR1|=SDA;}
#define	I2C_SCL(x)	{if(x)DIR0&=~SCL;else DIR0|=SCL;}
#define	I2C_DELAY	{udelay(5);}
#define	I2C_ACTIVE	{DIR1|=SDA;}
#define	I2C_TRISTATE    {DIR1&=~SDA;}
#endif
#endif

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x20000000
#define CFG_INIT_RAM_END	0x1000	/* End of used area in internal SRAM	*/
#define CFG_GBL_DATA_SIZE	64	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0xFFC40000	/* Address of Environment Sector*/
#define CFG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x10000 /* see README - env sector total size	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_SIZE		16		/* SDRAM size in MB */
#define CFG_FLASH_BASE		0xffc00000

#if 0 /* test-only */
#define CONFIG_PRAM             512 /* test-only for SDRAM problem!!!!!!!!!!!!!!!!!!!! */
#endif

#define CFG_MONITOR_BASE	(CFG_FLASH_BASE + 0x400)

#define CFG_MONITOR_LEN		0x20000
#define CFG_MALLOC_LEN		(1 * 1024*1024)	/* Reserve 1 MB for malloc()	*/
#define CFG_BOOTPARAMS_LEN	64*1024

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_WORD_SIZE	unsigned short	/* flash word size (width)	*/
#define CFG_FLASH_ADDR0		0x5555	/* 1st address for flash config cycles	*/
#define CFG_FLASH_ADDR1		0x2AAA	/* 2nd address for flash config cycles	*/
/*
 * The following defines are added for buggy IOP480 byte interface.
 * All other boards should use the standard values (CPCI405 etc.)
 */
#define CFG_FLASH_READ0		0x0000	/* 0 is standard			*/
#define CFG_FLASH_READ1		0x0001	/* 1 is standard			*/
#define CFG_FLASH_READ2		0x0002	/* 2 is standard			*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */

/* CS0 - AMD Flash, address 0xffc00000 */
#define	CFG_CSAR0               0xffc0
#define	CFG_CSCR0               0x1980          /* WS=0110, AA=1, PS=10         */
/** Note: There is a CSMR0/DRAM vector problem, need to disable C/I ***/
#define	CFG_CSMR0               0x003f0021      /* 4MB, AA=0, WP=0, C/I=1, V=1  */

/* CS1 - FPGA, address 0xe0000000 */
#define	CFG_CSAR1               0xe000
#define	CFG_CSCR1               0x0d80          /* WS=0011, AA=1, PS=10         */
#define	CFG_CSMR1               0x00010001      /* 128kB, AA=0, WP=0, C/I=0, V=1*/

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define	CFG_GPIO_FUNC           0x00000008      /* Set gpio pins: none          */
#define	CFG_GPIO1_FUNC          0x00df00f0      /* 36-39(SWITCH),48-52(FPGAs),54*/
#define	CFG_GPIO_EN             0x00000008      /* Set gpio output enable       */
#define	CFG_GPIO1_EN            0x00c70000      /* Set gpio output enable       */
#define	CFG_GPIO_OUT            0x00000008      /* Set outputs to default state */
#define	CFG_GPIO1_OUT           0x00c70000      /* Set outputs to default state */

#define CFG_GPIO1_LED           0x00400000      /* user led                     */

/*-----------------------------------------------------------------------
 * FPGA stuff
 */
#define CFG_FPGA_SPARTAN2	1	    /* using Xilinx Spartan 2 now    */
#define CFG_FPGA_MAX_SIZE	512*1024    /* 512kByte is enough for XC2S200*/

/* FPGA program pin configuration */
#define CFG_FPGA_PRG		0x00010000  /* FPGA program pin (ppc output) */
#define CFG_FPGA_CLK		0x00040000  /* FPGA clk pin (ppc output)     */
#define CFG_FPGA_DATA		0x00020000  /* FPGA data pin (ppc output)    */
#define CFG_FPGA_INIT		0x00080000  /* FPGA init pin (ppc input)     */
#define CFG_FPGA_DONE		0x00100000  /* FPGA done pin (ppc input)     */

#endif	/* _TASREG_H */
