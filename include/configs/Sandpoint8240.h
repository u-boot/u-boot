/*
 * (C) Copyright 2001-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#define CONFIG_MPC8240		1
#define CONFIG_SANDPOINT	1

#if 0
#define USE_DINK32		1
#else
#undef USE_DINK32
#endif

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run net_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"net_self=tftp ${kernel_addr} ${bootfile};"			\
		"tftp ${ramdisk_addr} ${ramdisk};"			\
		"run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp ${kernel_addr} ${bootfile};"			\
		"run nfsargs addip;bootm\0"				\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"bootfile=/tftpboot/SP8240/uImage\0"				\
	"ramdisk=/tftpboot/SP8240/uRamdisk\0"				\
	"kernel_addr=200000\0"						\
	"ramdisk_addr=400000\0"						\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"


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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SNTP


#define CONFIG_DRAM_SPEED	100		/* MHz				*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		1		/* undef to save memory		*/
#define CONFIG_SYS_PROMPT		"=> "		/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size	*/
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR		0x00100000	/* default load address		*/
#define CONFIG_SYS_HZ			1000		/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_PCI				/* include pci support		*/
#undef CONFIG_PCI_PNP

#define CONFIG_NET_MULTI			/* Multi ethernet cards support */

#define CONFIG_EEPRO100
#define CONFIG_SYS_RX_ETH_BUFFER	8               /* use 8 rx buffer on eepro100  */

#define PCI_ENET0_IOADDR	0x80000000
#define PCI_ENET0_MEMADDR	0x80000000
#define	PCI_ENET1_IOADDR	0x81000000
#define	PCI_ENET1_MEMADDR	0x81000000


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_MAX_RAM_SIZE	0x10000000

#define CONFIG_SYS_RESET_ADDRESS	0xFFF00100

#if defined (USE_DINK32)
#define CONFIG_SYS_MONITOR_LEN		0x00030000
#define CONFIG_SYS_MONITOR_BASE	0x00090000
#define CONFIG_SYS_RAMBOOT		1
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_INIT_RAM_END	0x10000
#define CONFIG_SYS_GBL_DATA_SIZE	256  /* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#else
#undef	CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_MONITOR_LEN		0x00030000
#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE

/*#define CONFIG_SYS_GBL_DATA_SIZE    256*/
#define CONFIG_SYS_GBL_DATA_SIZE	128

#define CONFIG_SYS_INIT_RAM_ADDR     0x40000000
#define CONFIG_SYS_INIT_RAM_END      0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET  (CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)

#endif

#define CONFIG_SYS_FLASH_BASE		0xFFF00000
#if 0
#define CONFIG_SYS_FLASH_SIZE		(512 * 1024)	/* sandpoint has tiny eeprom	*/
#else
#define CONFIG_SYS_FLASH_SIZE		(1024 * 1024)	/* Unity has onboard 1MByte flash */
#endif
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0x00004000	/* Offset of Environment Sector */
#define CONFIG_ENV_SIZE		0x00002000	/* Total Size of Environment Sector */

#define CONFIG_SYS_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()	*/

#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x04000000	/* 0 ... 32 MB in DRAM		*/

#define CONFIG_SYS_EUMB_ADDR		0xFC000000

#define CONFIG_SYS_ISA_MEM		0xFD000000
#define CONFIG_SYS_ISA_IO		0xFE000000

#define CONFIG_SYS_FLASH_RANGE_BASE	0xFF000000	/* flash memory address range	*/
#define CONFIG_SYS_FLASH_RANGE_SIZE	0x01000000
#define FLASH_BASE0_PRELIM	0xFFF00000	/* sandpoint flash		*/
#define FLASH_BASE1_PRELIM	0xFF000000	/* PMC onboard flash		*/

/*
 * select i2c support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#define CONFIG_HARD_I2C		1		/* To enable I2C support	*/
#undef  CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CONFIG_SYS_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F

#ifdef CONFIG_SOFT_I2C
#error "Soft I2C is not configured properly.  Please review!"
#define I2C_PORT		3               /* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE		(iop->pdir |=  0x00010000)
#define I2C_TRISTATE		(iop->pdir &= ~0x00010000)
#define I2C_READ		((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)		if(bit) iop->pdat |=  0x00010000; \
				else    iop->pdat &= ~0x00010000
#define I2C_SCL(bit)		if(bit) iop->pdat |=  0x00020000; \
				else    iop->pdat &= ~0x00020000
#define I2C_DELAY		udelay(5)	/* 1/4 I2C clock duration */
#endif /* CONFIG_SOFT_I2C */


#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57		/* EEPROM IS24C02		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1		/* Bytes of address		*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3	/* write page size		*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* takes up to 10 msec		*/


#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_FLASH_BANKS		{ FLASH_BASE0_PRELIM , FLASH_BASE1_PRELIM }

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */


#define CONFIG_WINBOND_83C553	1	/*has a winbond bridge			*/
#define CONFIG_SYS_USE_WINBOND_IDE	0	/*use winbond 83c553 internal IDE ctrlr */
#define CONFIG_SYS_WINBOND_ISA_CFG_ADDR    0x80005800	/*pci-isa bridge config addr	*/
#define CONFIG_SYS_WINBOND_IDE_CFG_ADDR    0x80005900	/*ide config addr		*/

#define CONFIG_SYS_IDE_MAXBUS		2   /* max. 2 IDE busses	*/
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

/*
 * NS87308 Configuration
 */
#define CONFIG_NS87308			/* Nat Semi super-io controller on ISA bus */

#define CONFIG_SYS_NS87308_BADDR_10	1

#define CONFIG_SYS_NS87308_DEVS	( CONFIG_SYS_NS87308_UART1   | \
				  CONFIG_SYS_NS87308_UART2   | \
				  CONFIG_SYS_NS87308_POWRMAN | \
				  CONFIG_SYS_NS87308_RTC_APC )

#undef  CONFIG_SYS_NS87308_PS2MOD

#define CONFIG_SYS_NS87308_CS0_BASE	0x0076
#define CONFIG_SYS_NS87308_CS0_CONF	0x30
#define CONFIG_SYS_NS87308_CS1_BASE	0x0075
#define CONFIG_SYS_NS87308_CS1_CONF	0x30
#define CONFIG_SYS_NS87308_CS2_BASE	0x0074
#define CONFIG_SYS_NS87308_CS2_CONF	0x30

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_NS16550_CLK		1843200

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_ISA_IO + CONFIG_SYS_NS87308_UART1_BASE)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_ISA_IO + CONFIG_SYS_NS87308_UART2_BASE)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_CLK_FREQ  33000000	/* external frequency to pll */
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER  1

#define CONFIG_SYS_ROMNAL		7	/*rom/flash next access time		*/
#define CONFIG_SYS_ROMFAL		11	/*rom/flash access time			*/

#define CONFIG_SYS_REFINT	430	/* no of clock cycles between CBR refresh cycles */

/* the following are for SDRAM only*/
#define CONFIG_SYS_BSTOPRE	121	/* Burst To Precharge, sets open page interval */
#define CONFIG_SYS_REFREC		8	/* Refresh to activate interval		*/
#define CONFIG_SYS_RDLAT		4	/* data latency from read command	*/
#define CONFIG_SYS_PRETOACT		3	/* Precharge to activate interval	*/
#define CONFIG_SYS_ACTTOPRE		5	/* Activate to Precharge interval	*/
#define CONFIG_SYS_ACTORW		3	/* Activate to R/W			*/
#define CONFIG_SYS_SDMODE_CAS_LAT	3	/* SDMODE CAS latency			*/
#define CONFIG_SYS_SDMODE_WRAP		0	/* SDMODE wrap type			*/
#define CONFIG_SYS_SDMODE_BURSTLEN	2	/* SDMODE Burst length 2=4, 3=8		*/

#define CONFIG_SYS_REGISTERD_TYPE_BUFFER   1

/* memory bank settings*/
/*
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
/*
 * Memory bank enable bitmask, specifying which of the banks defined above
 are actually present. MSB is for bank #7, LSB is for bank #0.
 */
#define CONFIG_SYS_BANK_ENABLE		0x01

#define CONFIG_SYS_ODCR		0xff	/* configures line driver impedances,	*/
					/* see 8240 book for bit definitions	*/
#define CONFIG_SYS_PGMAX		0x32	/* how long the 8240 retains the	*/
					/* currently accessed page in memory	*/
					/* see 8240 book for details		*/

/* SDRAM 0 - 256MB */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* stack in DCACHE @ 1GB (no backing mem) */
#if defined(USE_DINK32)
#define CONFIG_SYS_IBAT1L	(0x40000000 | BATL_PP_00 )
#define CONFIG_SYS_IBAT1U	(0x40000000 | BATU_BL_128K )
#else
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#endif

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

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	20	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8240 CPU			*/
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/


/* values according to the manual */

#define CONFIG_DRAM_50MHZ	1
#define CONFIG_SDRAM_50MHZ

#undef	NR_8259_INTS
#define NR_8259_INTS		1


#define CONFIG_DISK_SPINUP_TIME 1000000


#endif	/* __CONFIG_H */
