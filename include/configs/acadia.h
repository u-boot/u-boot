/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * acadia.h - configuration for AMCC Acadia (405EZ)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_ACADIA		1		/* Board is Acadia	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_405EZ		1		/* Specifc 405EZ support*/
/* Detect Acadia PLL input clock automatically via CPLD bit		*/
#define CONFIG_SYS_CLK_FREQ    ((in8(CFG_CPLD_BASE + 0) == 0x0c) ? \
				66666666 : 33333000)

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_F	1		/* Call misc_init_f	*/

#define CONFIG_NO_SERIAL_EEPROM
/*#undef CONFIG_NO_SERIAL_EEPROM*/

#ifdef CONFIG_NO_SERIAL_EEPROM
/*----------------------------------------------------------------------------
 * PLL settings for 266MHz CPU, 133MHz PLB/SDRAM, 66MHz EBC, 33MHz PCI,
 * assuming a 66MHz input clock to the 405EZ.
 *---------------------------------------------------------------------------*/
/* #define PLLMR0_100_100_12 */
#define PLLMR0_200_133_66
/* #define PLLMR0_266_160_80 */
/* #define PLLMR0_333_166_83 */
#endif

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(256 * 1024)/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(512 * 1024)/* Reserve 512 kB for malloc()	*/

#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xfe000000
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_CPLD_BASE		0x80000000
#define CFG_NAND_ADDR		0xd0000000
#define CFG_USB_HOST		0xef603000	/* USB OHCI 1.1 controller	*/

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
#define CFG_TEMP_STACK_OCM	1		/* OCM as init ram	*/

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xf8000000
#define CFG_OCM_DATA_SIZE	0x4000			/* 16K of onchip SRAM		*/
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR	/* inside of SRAM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE	/* End of used area in RAM	*/

#define CFG_GBL_DATA_SIZE	128			/* size for initial data	*/
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CFG_EXT_SERIAL_CLOCK			/* external serial clock */
#define CFG_BASE_BAUD		691200
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI     1

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CFG_ENV_IS_IN_NAND	1	/* use NAND for environment vars	*/
#define CFG_ENV_IS_EMBEDDED	1	/* use embedded environment */
#endif

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_FLASH_CFI			/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/

#define CFG_FLASH_BANKS_LIST    {CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define	_CFG_CMD_INCLUDE	(CFG_CMD_ALL)
#else
#define	CFG_NO_FLASH		1	/* No NOR on Acadia when NAND-booting	*/
#define	_CFG_CMD_INCLUDE	((CFG_CMD_ALL) & ~(CFG_CMD_FLASH | CFG_CMD_IMLS))
#endif

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x40000 /* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif

/*
 * IPL (Initial Program Loader, integrated inside CPU)
 * Will load first 4k from NAND (SPL) into cache and execute it from there.
 *
 * SPL (Secondary Program Loader)
 * Will load special U-Boot version (NUB) from NAND and execute it. This SPL
 * has to fit into 4kByte. It sets up the CPU and configures the SDRAM
 * controller and the NAND controller so that the special U-Boot image can be
 * loaded from NAND to SDRAM.
 *
 * NUB (NAND U-Boot)
 * This NAND U-Boot (NUB) is a special U-Boot version which can be started
 * from RAM. Therefore it mustn't (re-)configure the SDRAM controller.
 *
 * On 440EPx the SPL is copied to SDRAM before the NAND controller is
 * set up. While still running from cache, I experienced problems accessing
 * the NAND controller.	sr - 2006-08-25
 */
#define CFG_NAND_BOOT_SPL_SRC	0xfffff000	/* SPL location			*/
#define CFG_NAND_BOOT_SPL_SIZE	(4 << 10)	/* SPL size			*/
#define CFG_NAND_BOOT_SPL_DST	(CFG_OCM_DATA_ADDR + (16 << 10)) /* Copy SPL here*/
#define CFG_NAND_U_BOOT_DST	0x01000000	/* Load NUB to this addr	*/
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST /* Start NUB from this addr	*/
#define CFG_NAND_BOOT_SPL_DELTA	(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CFG_NAND_U_BOOT_OFFS	(16 << 10)	/* Offset to RAM U-Boot image	*/
#define CFG_NAND_U_BOOT_SIZE	(384 << 10)	/* Size of RAM U-Boot image	*/

/*
 * Now the NAND chip has to be defined (no autodetection used!)
 */
#define CFG_NAND_PAGE_SIZE	512		/* NAND chip page size		*/
#define CFG_NAND_BLOCK_SIZE	(16 << 10)	/* NAND chip block size		*/
#define CFG_NAND_PAGE_COUNT	32		/* NAND chip page count		*/
#define CFG_NAND_BAD_BLOCK_POS	5		/* Location of bad block marker	*/
#undef CFG_NAND_4_ADDR_CYCLE			/* No fourth addr used (<=32MB)	*/

#define CFG_NAND_ECCSIZE	256
#define CFG_NAND_ECCBYTES	3
#define CFG_NAND_ECCSTEPS	(CFG_NAND_PAGE_SIZE / CFG_NAND_ECCSIZE)
#define CFG_NAND_OOBSIZE	16
#define CFG_NAND_ECCTOTAL	(CFG_NAND_ECCBYTES * CFG_NAND_ECCSTEPS)
#define CFG_NAND_ECCPOS		{0, 1, 2, 3, 6, 7}

#ifdef CFG_ENV_IS_IN_NAND
/*
 * For NAND booting the environment is embedded in the U-Boot image. Please take
 * look at the file board/amcc/sequoia/u-boot-nand.lds for details.
 */
#define CFG_ENV_SIZE		CFG_NAND_BLOCK_SIZE
#define CFG_ENV_OFFSET		(CFG_NAND_U_BOOT_OFFS + CFG_ENV_SIZE)
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET + CFG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * RAM (CRAM)
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_RAM		64		/* 64MB			*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1		/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	(0xa8>>1)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75		1		/* ON Semi's LM75	*/
#define CONFIG_DTT_AD7414	1		/* use AD7414		*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CFG_DTT_MAX_TEMP	70
#define CFG_DTT_LOW_TEMP	-30
#define CFG_DTT_HYSTERESIS	3

#if 0 /* test-only... */
/*-----------------------------------------------------------------------
 * SPI stuff - Define to include SPI control
 *-----------------------------------------------------------------------
 */
#define CONFIG_SPI
#endif

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_MII		1	/* MII PHY management		*/
#define	CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_NET_MULTI	1
#define CFG_RX_ETH_BUFFER	16	/* # of rx buffers & descriptors*/

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=acadia\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
		"bootm\0"						\
	"rootpath=/opt/eldk/ppc_4xx\0"				\
	"bootfile=acadia/uImage\0"					\
	"kernel_addr=fff10000\0"					\
	"ramdisk_addr=fff20000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 200000 acadia/u-boot.bin\0"				\
	"update=protect off fffc0000 ffffffff;era fffc0000 ffffffff;"	\
		"cp.b ${fileaddr} fffc0000 ${filesize};"		\
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
	"nload=tftp 200000 acadia/u-boot-nand.bin\0"			\
	"nupdate=nand erase 0 60000;nand write 200000 0 60000;"		\
		"setenv filesize;saveenv\0"				\
	"nupd=run nload nupdate\0"					\
	"kozio=bootm ffc60000\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#define CONFIG_SUPPORT_VFAT

#define CONFIG_COMMANDS	((CONFIG_CMD_DFL & _CFG_CMD_INCLUDE)	|	\
			 CFG_CMD_ASKENV	|				\
			 CFG_CMD_DHCP	|				\
			 CFG_CMD_DTT	|				\
			 CFG_CMD_DIAG	|				\
			 CFG_CMD_EEPROM	|				\
			 CFG_CMD_ELF	|				\
			 CFG_CMD_FAT	|				\
			 CFG_CMD_I2C	|				\
			 CFG_CMD_IRQ	|				\
			 CFG_CMD_MII	|				\
			 CFG_CMD_NAND	|				\
			 CFG_CMD_NET	|				\
			 CFG_CMD_NFS	|				\
			 CFG_CMD_PCI	|				\
			 CFG_CMD_PING	|				\
			 CFG_CMD_REGINFO |				\
			 CFG_CMD_USB)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG					/* watchdog disabled		*/

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	        16	/* max number of command args	*/
#define CFG_BARGSIZE	        CFG_CBSIZE /* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000  /* default load address	*/
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * NAND FLASH
 *----------------------------------------------------------------------*/
#define CFG_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE		(CFG_NAND_ADDR + CFG_NAND_CS)
#define CFG_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384		/* For AMCC 405EZ CPU		*/
#define CFG_CACHELINE_SIZE	32		/* ...				*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5		/* log base 2 of the above value*/
#endif

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_NAND_CS		3
/* Memory Bank 0 (Flash) initialization						*/
#define CFG_EBC_PB0AP		0x03337200
#define CFG_EBC_PB0CR		0xfe0bc000

/* Memory Bank 3 (NAND-FLASH) initialization					*/
#define CFG_EBC_PB3AP		0x018003c0
#define CFG_EBC_PB3CR		(CFG_NAND_ADDR | 0x1c000)

/* Just initial configuration for CRAM. Will be changed in memory.c to sync mode*/
/* Memory Bank 1 (CRAM) initialization						*/
#define CFG_EBC_PB1AP		0x030400c0
#define CFG_EBC_PB1CR		0x000bc000

/* Memory Bank 2 (CRAM) initialization						*/
#define CFG_EBC_PB2AP		0x030400c0
#define CFG_EBC_PB2CR		0x020bc000
#else
#define CFG_NAND_CS		0		/* NAND chip connected to CSx	*/
/* Memory Bank 0 (NAND-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x018003c0
#define CFG_EBC_PB0CR		(CFG_NAND_ADDR | 0x1c000)

/*
 * When NAND-booting the CRAM EBC setup must be done in sync mode, since the
 * NAND-SPL already initialized the CRAM and EBC to sync mode.
 */
/* Memory Bank 1 (CRAM) initialization						*/
#define CFG_EBC_PB1AP		0x9C0201C0
#define CFG_EBC_PB1CR		0x000bc000

/* Memory Bank 2 (CRAM) initialization						*/
#define CFG_EBC_PB2AP		0x9C0201C0
#define CFG_EBC_PB2CR		0x020bc000
#endif

/* Memory Bank 4 (CPLD) initialization						*/
#define CFG_EBC_PB4AP		0x04006000
#define CFG_EBC_PB4CR		(CFG_CPLD_BASE | 0x18000)

#define CFG_EBC_CFG		0xf8400000

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CFG_GPIO_CRAM_CLK	8
#define CFG_GPIO_CRAM_WAIT	9		/* GPIO-In		*/
#define CFG_GPIO_CRAM_ADV	10
#define CFG_GPIO_CRAM_CRE	(32 + 21)	/* GPIO-Out		*/

/*-----------------------------------------------------------------------
 * Definitions for GPIO_0 setup (PPC405EZ specific)
 *
 * GPIO0[0-2]	- External Bus Controller CS_4 - CS_6 Outputs
 * GPIO0[3]	- NAND FLASH Controller CE3 (NFCE3) Output
 * GPIO0[4]	- External Bus Controller Hold Input
 * GPIO0[5]	- External Bus Controller Priority Input
 * GPIO0[6]	- External Bus Controller HLDA Output
 * GPIO0[7]	- External Bus Controller Bus Request Output
 * GPIO0[8]	- CRAM Clk Output
 * GPIO0[9]	- External Bus Controller Ready Input
 * GPIO0[10]	- CRAM Adv Output
 * GPIO0[11-24]	- NAND Flash Control Data -> Bypasses GPIO when enabled
 * GPIO0[25]	- External DMA Request Input
 * GPIO0[26]	- External DMA EOT I/O
 * GPIO0[25]	- External DMA Ack_n Output
 * GPIO0[17-23]	- External Interrupts IRQ0 - IRQ6 inputs
 * GPIO0[28-30]	- Trace Outputs / PWM Inputs
 * GPIO0[31]	- PWM_8 I/O
 */
#define CFG_GPIO0_TCR		0xC0A00000
#define CFG_GPIO0_OSRL		0x50004400
#define CFG_GPIO0_OSRH		0x02000055
#define CFG_GPIO0_ISR1L		0x00001000
#define CFG_GPIO0_ISR1H		0x00000055
#define CFG_GPIO0_TSRL		0x02000000
#define CFG_GPIO0_TSRH		0x00000055

/*-----------------------------------------------------------------------
 * Definitions for GPIO_1 setup (PPC405EZ specific)
 *
 * GPIO1[0-6]	- PWM_9 to PWM_15 I/O
 * GPIO1[7]	- PWM_DIV_CLK (Out) / IRQ4 Input
 * GPIO1[8]	- TS5 Output / DAC_IP_TRIG Input
 * GPIO1[9]	- TS6 Output / ADC_IP_TRIG Input
 * GPIO1[10-12]	- UART0 Control Inputs
 * GPIO1[13]	- UART0_DTR_N Output/IEEE_1588_TS Output/TMRCLK Input
 * GPIO1[14]	- UART0_RTS_N Output/SPI_SS_2_N Output
 * GPIO1[15]	- SPI_SS_3_N Output/UART0_RI_N Input
 * GPIO1[16]	- SPI_SS_1_N Output
 * GPIO1[17-20]	- Trace Output/External Interrupts IRQ0 - IRQ3 inputs
 */
#define CFG_GPIO1_TCR		0xFFFF8414
#define CFG_GPIO1_OSRL		0x40000110
#define CFG_GPIO1_OSRH		0x55455555
#define CFG_GPIO1_ISR1L		0x15555445
#define CFG_GPIO1_ISR1H		0x00000000
#define CFG_GPIO1_TSRL		0x00000000
#define CFG_GPIO1_TSRH		0x00000000

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
