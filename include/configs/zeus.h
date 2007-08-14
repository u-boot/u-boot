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
 * zeus.h - configuration for Zeus board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_ZEUS		1		/* Board is Zeus	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_405EP		1		/* Specifc 405EP support*/

#define CONFIG_SYS_CLK_FREQ     33000000 /* external frequency to pll   */

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/

#define PLLMR0_DEFAULT		PLLMR0_333_111_55_111
#define PLLMR1_DEFAULT		PLLMR1_333_111_55_111

#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

#define CONFIG_OVERWRITE_ETHADDR_ONCE	1

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0x01	/* PHY address			*/
#define CONFIG_HAS_ETH1		1
#define CONFIG_PHY1_ADDR	0x11	/* EMAC1 PHY address		*/
#define CONFIG_NET_MULTI	1
#define CFG_RX_ETH_BUFFER	16	/* Number of ethernet rx buffers & descriptors */
#define CONFIG_PHY_RESET	1
#define CONFIG_PHY_RESET_DELAY	300	/* PHY RESET recovery delay	*/

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL	|	\
				 CFG_CMD_ASKENV	|	\
				 CFG_CMD_CACHE	|	\
				 CFG_CMD_DHCP	|	\
				 CFG_CMD_DIAG	|	\
				 CFG_CMD_EEPROM |	\
				 CFG_CMD_ELF    |	\
				 CFG_CMD_I2C    |	\
				 CFG_CMD_IRQ	|	\
				 CFG_CMD_LOG	|	\
				 CFG_CMD_MII	|	\
				 CFG_CMD_NET	|	\
				 CFG_CMD_NFS	|	\
				 CFG_CMD_PING	|	\
				 CFG_CMD_REGINFO)

/* POST support */
#define CONFIG_POST		(CFG_POST_MEMORY   | \
				 CFG_POST_CPU	   | \
				 CFG_POST_CACHE	   | \
				 CFG_POST_UART	   | \
				 CFG_POST_ETHER)

#define CFG_POST_ETHER_EXT_LOOPBACK	/* eth POST using ext loopack connector	*/

/* Define here the base-addresses of the UARTs to test in POST */
#define CFG_POST_UART_TABLE	{UART0_BASE}

#define CONFIG_LOGBUFFER
#define CFG_POST_CACHE_ADDR	0x00800000 /* free virtual address	*/

#define CFG_CONSOLE_IS_IN_ENV /* Otherwise it catches logbuffer as output */

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*-----------------------------------------------------------------------
 * SDRAM
 *----------------------------------------------------------------------*/
/*
 * SDRAM configuration (please see cpu/ppc/sdram.[ch])
 */
#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0 */
#define CONFIG_SDRAM_BANK1	1	/* init onboard SDRAM bank 1 */

/* SDRAM timings used in datasheet */
#define CFG_SDRAM_CL            3	/* CAS latency */
#define CFG_SDRAM_tRP           20	/* PRECHARGE command period */
#define CFG_SDRAM_tRC           66	/* ACTIVE-to-ACTIVE command period */
#define CFG_SDRAM_tRCD          20	/* ACTIVE-to-READ delay */
#define CFG_SDRAM_tRFC		66	/* Auto refresh period */

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef	CFG_EXT_SERIAL_CLOCK			/* external serial clock */
#define CFG_BASE_BAUD		691200
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

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

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1		/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

/* these are for the ST M24C02 2kbit serial i2c eeprom */
#define CFG_I2C_EEPROM_ADDR	0x50		/* base address */
#define CFG_I2C_EEPROM_ADDR_LEN	1		/* bytes of address */
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07

#define CFG_EEPROM_PAGE_WRITE_ENABLE	1	/* write eeprom in pages */
#define CFG_EEPROM_PAGE_WRITE_BITS	3	/* 8 byte write page size */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */

/*
 * The layout of the I2C EEPROM, used for bootstrap setup and for board-
 * specific values, like ethaddr... that can be restored via the sw-reset
 * button
 */
#define FACTORY_RESET_I2C_EEPROM	0x50
#define FACTORY_RESET_ENV_OFFS		0x80
#define FACTORY_RESET_ENV_SIZE		0x80

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFF000000
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/
#define CFG_MONITOR_BASE	(-CFG_MONITOR_LEN)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_FLASH_CFI				/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash	*/

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000	/* size of one complete sector		*/
#define CFG_ENV_ADDR		((-CFG_MONITOR_LEN)-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384	/* For IBM 405EP CPU			*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory (OCM) for temperary stack until sdram is tested */
#define CFG_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of OCM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/

#define CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
/* reserve some memory for POST and BOOT limit info */
#define CFG_INIT_SP_OFFSET	(CFG_GBL_DATA_OFFSET - 16)

/* extra data in OCM */
#define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 4)
#define CFG_POST_MAGIC		(CFG_OCM_DATA_ADDR + CFG_GBL_DATA_OFFSET - 8)
#define CFG_POST_VAL		(CFG_OCM_DATA_ADDR + CFG_GBL_DATA_OFFSET - 12)

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash 16M) initialization					*/
#define CFG_EBC_PB0AP		0x05815600
#define CFG_EBC_PB0CR		0xFF09A000  /* BAS=0xFF0,BS=16MB,BU=R/W,BW=16bit  */

/*-----------------------------------------------------------------------
 * Definitions for GPIO setup (PPC405EP specific)
 *
 * GPIO0[0]     - External Bus Controller BLAST output
 * GPIO0[1-9]   - Instruction trace outputs
 * GPIO0[10-13] - External Bus Controller CS_1 - CS_4 outputs
 * GPIO0[14-16] - External Bus Controller ABUS3-ABUS5 outputs
 * GPIO0[17-23] - External Interrupts IRQ0 - IRQ6 inputs
 * GPIO0[24-27] - UART0 control signal inputs/outputs
 * GPIO0[28-29] - UART1 data signal input/output
 * GPIO0[30-31] - EMAC0 and EMAC1 reject packet inputs
 */
#define CFG_GPIO0_OSRH		0x15555550	/* Chip selects */
#define CFG_GPIO0_OSRL		0x00000110	/* UART_DTR-pin 27 alt out */
#define CFG_GPIO0_ISR1H		0x10000041	/* Pin 2, 12 is input */
#define CFG_GPIO0_ISR1L		0x15505440	/* OUT: LEDs 22/23; IN: pin12,2, NVALID# */
#define CFG_GPIO0_TSRH		0x00000000
#define CFG_GPIO0_TSRL		0x00000000
#define CFG_GPIO0_TCR		0xBFF68317	/* 3-state OUT: 22/23/29; 12,2 is not 3-state */
#define CFG_GPIO0_ODR		0x00000000

#define CFG_GPIO_SW_RESET	1
#define CFG_GPIO_ZEUS_PE	12
#define CFG_GPIO_LED_RED	22
#define CFG_GPIO_LED_GREEN	23

/* Time in milli-seconds */
#define CFG_TIME_POST		5000
#define CFG_TIME_FACTORY_RESET	10000

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02		/* Software reboot			*/

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/* ENVIRONMENT VARS */

#define CONFIG_PREBOOT		"echo;echo Welcome to Bulletendpoints board v1.1;echo"
#define CONFIG_IPADDR		192.168.1.10
#define CONFIG_SERVERIP		192.168.1.100
#define CONFIG_GATEWAYIP	192.168.1.100
#define CONFIG_ETHADDR		50:00:00:00:06:00
#define CONFIG_ETH1ADDR		50:00:00:00:06:01
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled        */
#else
#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds */
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"logversion=2\0"                                                \
	"hostname=zeus\0"						\
	"netdev=eth0\0"							\
	"ethact=ppc_4xx_eth0\0"						\
	"netmask=255.255.255.0\0"					\
	"ramdisk_size=50000\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw"			\
		" nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw"			\
		" ramdisk=${ramdisk_size}\0"				\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
	        ":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,"		\
		"${baudrate}\0"						\
	"net_nfs=tftp ${kernel_mem_addr} ${file_kernel};"		\
		"run nfsargs addip addtty;bootm\0"			\
	"net_ram=tftp ${kernel_mem_addr} ${file_kernel};"		\
		"tftp ${ramdisk_mem_addr} ${file_fs};"			\
		"run ramargs addip addtty;"				\
		"bootm ${kernel_mem_addr} ${ramdisk_mem_addr}\0"	\
	"rootpath=/target_fs/zeus\0"					\
	"kernel_fl_addr=ff000000\0"					\
	"kernel_mem_addr=200000\0"					\
	"ramdisk_fl_addr=ff300000\0"					\
	"ramdisk_mem_addr=4000000\0"					\
	"uboot_fl_addr=fffc0000\0"					\
	"uboot_mem_addr=100000\0"					\
	"file_uboot=/zeus/u-boot.bin\0"					\
	"tftp_uboot=tftp 100000 ${file_uboot}\0"			\
	"update_uboot=protect off fffc0000 ffffffff;"			\
		"era fffc0000 ffffffff;cp.b 100000 fffc0000 40000;"	\
		"protect on fffc0000 ffffffff\0"			\
	"upd_uboot=run tftp_uboot;run update_uboot\0"			\
	"file_kernel=/zeus/uImage_ba\0"					\
	"tftp_kernel=tftp 100000 ${file_kernel}\0"			\
	"update_kernel=protect off ff000000 ff17ffff;"			\
		"era ff000000 ff17ffff;cp.b 100000 ff000000 180000\0"	\
	"upd_kernel=run tftp_kernel;run update_kernel\0"		\
	"file_fs=/zeus/rootfs_ba.img\0"					\
	"tftp_fs=tftp 100000 ${file_fs}\0"				\
	"update_fs=protect off ff300000 ff87ffff;era ff300000 ff87ffff;"\
	        "cp.b 100000 ff300000 580000\0"				\
	"upd_fs=run tftp_fs;run update_fs\0"				\
	"bootcmd=chkreset;run ramargs addip addtty addmisc;"		\
		"bootm ${kernel_fl_addr} ${ramdisk_fl_addr}\0"		\
	""

#endif	/* __CONFIG_H */
