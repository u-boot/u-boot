/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC824X		1
/* #define CONFIG_MPC8240	   1 */
#define CONFIG_MPC8245		1
#define CONFIG_EXALION		1

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

#if defined (CONFIG_MPC8240)
    /* #warning	 ---------- eXalion with MPC8240 --------------- */
#elif defined (CONFIG_MPC8245)
    /* #warning	 ++++++++++ eXalion with MPC8245 +++++++++++++++ */
#elif defined (CONFIG_MPC8245) && defined (CONFIG_MPC8245)
#error #### Both types of MPC824x defined (CONFIG_8240 and CONFIG_8245)
#else
#error #### Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif
/* older kernels need clock in MHz newer in Hz */
					/* #define CONFIG_CLOCKS_IN_MHZ 1 */ /* clocks passsed to Linux in MHz	    */
#undef CONFIG_CLOCKS_IN_MHZ

#define CONFIG_BOOTDELAY	10


						    /*#define CONFIG_DRAM_SPEED	      66   */ /* MHz			     */

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

#define CONFIG_CMD_FLASH
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_PCI


/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		1	/* undef to save memory		*/
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size	*/
#define CONFIG_SYS_MAXARGS		8	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR		0x00100000	/* default load address		*/
#define CONFIG_MISC_INIT_R	1

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_MAX_RAM_SIZE	0x10000000	/* 1 GBytes - initdram() will	   */
					     /* return real value.		*/

#define CONFIG_SYS_RESET_ADDRESS	0xFFF00100

#undef	CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	    */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_DATA_SIZE	128

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - CONFIG_SYS_INIT_DATA_SIZE)

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET


#if defined (CONFIG_MPC8240)
#define CONFIG_SYS_FLASH_BASE	    0xFFE00000
#define CONFIG_SYS_FLASH_SIZE	    (2 * 1024 * 1024)	/* onboard 2MByte flash	    */
#elif defined (CONFIG_MPC8245)
#define CONFIG_SYS_FLASH_BASE	    0xFFC00000
#define CONFIG_SYS_FLASH_SIZE	    (4 * 1024 * 1024)	/* onboard 4MByte flash	    */
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	0x20000 /* Size of one Flash sector */
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE	/* Use one Flash sector for environment	*/
#define CONFIG_ENV_ADDR		0xFFFC0000
#define CONFIG_ENV_OFFSET		0	/* starting right at the beginning  */

#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

#define CONFIG_SYS_ALT_MEMTEST		1	/* use real memory test	    */
#define CONFIG_SYS_MEMTEST_START	0x00004000	/* memtest works on	    */
#define CONFIG_SYS_MEMTEST_END		0x02000000	/* 0 ... 32 MB in DRAM	    */

#define CONFIG_SYS_EUMB_ADDR		0xFC000000

/* #define CONFIG_SYS_ISA_MEM		   0xFD000000 */
#define CONFIG_SYS_ISA_IO		0xFE000000

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* Max number of flash banks	    */
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* Max number of sectors per flash  */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE
#define FLASH_BASE1_PRELIM	0


/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver		*/
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_FLASH_INCREMENT	0	/* there is only one bank		*/
#define CONFIG_SYS_FLASH_PROTECTION	1	/* use hardware protection		*/
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/


/*-----------------------------------------------------------------------
 * PCI stuff
 */
#define CONFIG_PCI		1	/* include pci support		*/
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#undef	CONFIG_PCI_PNP


#define CONFIG_EEPRO100		1

#define PCI_ENET0_MEMADDR	0x80000000	/* Intel 82559ER */
#define PCI_ENET0_IOADDR	0x80000000
#define PCI_ENET1_MEMADDR	0x81000000	/* Intel 82559ER */
#define PCI_ENET1_IOADDR	0x81000000
#define PCI_ENET2_MEMADDR	0x82000000	/* Broadcom BCM569xx */
#define PCI_ENET2_IOADDR	0x82000000
#define PCI_ENET3_MEMADDR	0x83000000	/* Broadcom BCM56xx */
#define PCI_ENET3_IOADDR	0x83000000

/*-----------------------------------------------------------------------
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550		1
#define CONFIG_SYS_NS16550_SERIAL	1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		38400

#define CONFIG_SYS_NS16550_REG_SIZE	1

#if (CONFIG_CONS_INDEX == 1)
#define CONFIG_SYS_NS16550_CLK		1843200 /* COM1 only !	*/
#else
#define CONFIG_SYS_NS16550_CLK ({ extern ulong get_bus_freq (ulong); get_bus_freq (0); })
#endif

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_ISA_IO + 0x3F8)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_EUMB_ADDR + 0x4500)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_EUMB_ADDR + 0x4600)

/*-----------------------------------------------------------------------
 * select i2c support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#undef	CONFIG_SYS_I2C_SOFT		/* I2C bit-banged		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*-----------------------------------------------------------------------
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_CLK_FREQ	33333333	/* external frequency to pll	*/
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER  2	/* for MPC8240 only		*/

				       /*#define CONFIG_133MHZ_DRAM	 1 */ /* For 133 MHZ DRAM only !!!!!!!!!!!    */

#if defined (CONFIG_MPC8245)
/* Bit-field values for PMCR2.							*/
#if defined (CONFIG_133MHZ_DRAM)
#define CONFIG_SYS_DLL_EXTEND		0x80	/* use DLL extended range - 133MHz only */
#define CONFIG_SYS_PCI_HOLD_DEL	0x20	/* delay and hold timing - 133MHz only	*/
#endif

/* Bit-field values for MIOCR1.							*/
#if !defined (CONFIG_133MHZ_DRAM)
#define CONFIG_SYS_DLL_MAX_DELAY	0x04	/*  longer DLL delay line - 66MHz only	*/
#endif
/* Bit-field values for MIOCR2.							*/
#define CONFIG_SYS_SDRAM_DSCD		0x20	/* SDRAM data in sample clock delay	*/
					/*	- note bottom 3 bits MUST be 0	*/
#endif

/* Bit-field values for MCCR1.							*/
#define CONFIG_SYS_ROMNAL		7	/*rom/flash next access time		*/
#define CONFIG_SYS_ROMFAL	       11	/*rom/flash access time			*/

/* Bit-field values for MCCR2.							*/
#define CONFIG_SYS_TSWAIT		0x5	/* Transaction Start Wait States timer	*/
#if defined (CONFIG_133MHZ_DRAM)
#define CONFIG_SYS_REFINT		1300	/* no of clock cycles between CBR	*/
#else  /* refresh cycles */
#define CONFIG_SYS_REFINT		750
#endif

/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.		*/
#if defined (CONFIG_133MHZ_DRAM)
#define CONFIG_SYS_BSTOPRE		1023
#else
#define CONFIG_SYS_BSTOPRE		250
#endif

/* Bit-field values for MCCR3.							*/
/* the following are for SDRAM only						*/

#if defined (CONFIG_133MHZ_DRAM)
#define CONFIG_SYS_REFREC		9	/* Refresh to activate interval		*/
#else
#define CONFIG_SYS_REFREC		5	/* Refresh to activate interval		*/
#endif
#if defined (CONFIG_MPC8240)
#define CONFIG_SYS_RDLAT		2	/* data latency from read command	*/
#endif

/* Bit-field values for MCCR4.	*/
#if defined (CONFIG_133MHZ_DRAM)
#define CONFIG_SYS_PRETOACT		3	/* Precharge to activate interval	*/
#define CONFIG_SYS_ACTTOPRE		7	/* Activate to Precharge interval	*/
#define CONFIG_SYS_ACTORW		5	/* Activate to R/W			*/
#define CONFIG_SYS_SDMODE_CAS_LAT	3	/* SDMODE CAS latency			*/
#else
#if 0
#define CONFIG_SYS_PRETOACT		2	/* Precharge to activate interval	*/
#define CONFIG_SYS_ACTTOPRE		3	/* Activate to Precharge interval	*/
#define CONFIG_SYS_ACTORW		3	/* Activate to R/W			*/
#define CONFIG_SYS_SDMODE_CAS_LAT	2	/* SDMODE CAS latency			*/
#endif
#define CONFIG_SYS_PRETOACT		2	/* Precharge to activate interval	*/
#define CONFIG_SYS_ACTTOPRE		5	/* Activate to Precharge interval	*/
#define CONFIG_SYS_ACTORW		3	/* Activate to R/W			*/
#define CONFIG_SYS_SDMODE_CAS_LAT	3	/* SDMODE CAS latency			*/
#endif
#define CONFIG_SYS_SDMODE_WRAP		0	/* SDMODE wrap type			*/
#define CONFIG_SYS_SDMODE_BURSTLEN	2	/* SDMODE Burst length 2=4, 3=8		*/
#define CONFIG_SYS_REGDIMM		0
#if defined (CONFIG_MPC8240)
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER   0
#elif defined (CONFIG_MPC8245)
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER   1
#define CONFIG_SYS_EXTROM		    0
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif


/*-----------------------------------------------------------------------
 memory bank settings
 * only bits 20-29 are actually used from these vales to set the
 * start/end address the upper two bits will be 0, and the lower 20
 * bits will be set to 0x00000 for a start address, or 0xfffff for an
 * end address
 */
#define CONFIG_SYS_BANK0_START		0x00000000
#define CONFIG_SYS_BANK0_END		(CONFIG_SYS_MAX_RAM_SIZE - 1)
#define CONFIG_SYS_BANK0_ENABLE	1
#define CONFIG_SYS_BANK1_START		0x3ff00000
#define CONFIG_SYS_BANK1_END		0x3fffffff
#define CONFIG_SYS_BANK1_ENABLE	0
#define CONFIG_SYS_BANK2_START		0x3ff00000
#define CONFIG_SYS_BANK2_END		0x3fffffff
#define CONFIG_SYS_BANK2_ENABLE	0
#define CONFIG_SYS_BANK3_START		0x3ff00000
#define CONFIG_SYS_BANK3_END		0x3fffffff
#define CONFIG_SYS_BANK3_ENABLE	0
#define CONFIG_SYS_BANK4_START		0x00000000
#define CONFIG_SYS_BANK4_END		0x00000000
#define CONFIG_SYS_BANK4_ENABLE	0
#define CONFIG_SYS_BANK5_START		0x00000000
#define CONFIG_SYS_BANK5_END		0x00000000
#define CONFIG_SYS_BANK5_ENABLE	0
#define CONFIG_SYS_BANK6_START		0x00000000
#define CONFIG_SYS_BANK6_END		0x00000000
#define CONFIG_SYS_BANK6_ENABLE	0
#define CONFIG_SYS_BANK7_START		0x00000000
#define CONFIG_SYS_BANK7_END		0x00000000
#define CONFIG_SYS_BANK7_ENABLE	0

/*-----------------------------------------------------------------------
 * Memory bank enable bitmask, specifying which of the banks defined above
 are actually present. MSB is for bank #7, LSB is for bank #0.
 */
#define CONFIG_SYS_BANK_ENABLE		0x01

#if defined (CONFIG_MPC8240)
#define CONFIG_SYS_ODCR		0xDF	/* configures line driver impedances,	*/
					/* see 8240 book for bit definitions	*/
#elif defined (CONFIG_MPC8245)
#if defined (CONFIG_133MHZ_DRAM)
#define CONFIG_SYS_ODCR		0xFE	/* configures line driver impedances - 133MHz	*/
#else
#define CONFIG_SYS_ODCR		0xDE	/* configures line driver impedances - 66MHz	*/
#endif
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif

#define CONFIG_SYS_PGMAX		0x32	/* how long the 8240 retains the	*/
					/* currently accessed page in memory	*/
					/* see 8240 book for details		*/

/*-----------------------------------------------------------------------
 * Block Address Translation (BAT) register settings.
 */
/* SDRAM 0 - 256MB */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* stack in DCACHE @ 1GB (no backing mem) */
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

/* PCI memory */
#define CONFIG_SYS_IBAT2L	(0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U	(0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

/* Flash, config addrs, etc */
#define CONFIG_SYS_IBAT3L	(0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT2L	CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT3L	CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/* values according to the manual */
#define CONFIG_DRAM_50MHZ	1
#define CONFIG_SDRAM_50MHZ

#undef	NR_8259_INTS
#define NR_8259_INTS		1

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 */
#define CONFIG_SYS_IDE_MAXBUS	    1	/* max. 2 IDE busses	*/
#define CONFIG_SYS_IDE_MAXDEVICE   (CONFIG_SYS_IDE_MAXBUS*1)	/* max. 2 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR   CONFIG_SYS_ISA_IO	/* base address */
#define CONFIG_SYS_ATA_IDE0_OFFSET 0x01F0	/* ide0 offste */
#define CONFIG_SYS_ATA_IDE1_OFFSET 0x0170	/* ide1 offset */
#define CONFIG_SYS_ATA_DATA_OFFSET 0	/* data reg offset  */
#define CONFIG_SYS_ATA_REG_OFFSET  0	/* reg offset */
#define CONFIG_SYS_ATA_ALT_OFFSET  0x200	/* alternate register offset */

#define CONFIG_ATAPI

#undef	CONFIG_IDE_8xx_DIRECT	/* no pcmcia interface required */
#undef	CONFIG_IDE_LED		/* no led for ide supported	*/
#undef	CONFIG_IDE_RESET	/* reset for ide supported...	 */
#undef	CONFIG_IDE_RESET_ROUTINE	/* with a special reset function */

/*-----------------------------------------------------------------------
 * DISK Partition support
 */
#define CONFIG_DOS_PARTITION

/*-----------------------------------------------------------------------
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#endif /* __CONFIG_H */
