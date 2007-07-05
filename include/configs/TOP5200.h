/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * modified for TOP5200-series by Reinhard Meyer, www.emk-elektronik.de
 *
 * TOP5200 differences from IceCube:
 * 1 FLASH Bank for one Chip only, up to 64 MB in 16 MB Banks
 *   bank switch controlled by TIMER_6(LSB) and TIMER_7(MSB) Pins
 * 1 SDRAM/DDRAM Bank up to 256 MB
 * local VPD I2C Bus is software driven and uses
 *   GPIO_WKUP_6 for SDA, GPIO_WKUP_7 for SCL
 * FLASH is re-located at 0xff000000
 * Internal regs are at 0xf0000000
 * Reset jumps to 0x00000100
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

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200		1	/* More exactly a MPC5200 */
#define CONFIG_TOP5200		1	/* ... on TOP5200 board - we need this for FEC.C */

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot	     */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		9600	/* ... at 9600 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }


#if defined (CONFIG_EVAL5200) || defined (CONFIG_LITE5200)
/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#  define CONFIG_PCI		1
#  define CONFIG_PCI_PNP		1
#  define CONFIG_PCI_SCAN_SHOW	1

#  define CONFIG_PCI_MEM_BUS	0x40000000
#  define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#  define CONFIG_PCI_MEM_SIZE	0x10000000

#  define CONFIG_PCI_IO_BUS	0x50000000
#  define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#  define CONFIG_PCI_IO_SIZE	0x01000000

#endif

/* USB */
#if defined (CONFIG_EVAL5200) || defined (CONFIG_LITE5200)

#  define CONFIG_USB_OHCI
#  define CONFIG_USB_CLOCK	0x0001bbbb
#  if defined (CONFIG_EVAL5200)
#    define CONFIG_USB_CONFIG	0x00005100
#  else
#    define CONFIG_USB_CONFIG	0x00001000
#  endif
#  define CONFIG_DOS_PARTITION
#  define CONFIG_USB_STORAGE

#endif

/* IDE */
#if defined (CONFIG_EVAL5200) || defined (CONFIG_LITE5200)
#  define CONFIG_DOS_PARTITION
#endif


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MII
#define CONFIG_CMD_REGINFO

#if defined (CONFIG_EVAL5200) || defined (CONFIG_LITE5200)
#define CONFIG_CMD_FAT
#define CONFIG_CMD_IDE
#define CONFIG_CMD_USB
#define CONFIG_CMD_PCI
#endif


/*
 * MUST be low boot - HIGHBOOT is not supported anymore
 */
#if (TEXT_BASE == 0xFF000000)		/* Boot low with 16 MB Flash */
#   define CFG_LOWBOOT		1
#   define CFG_LOWBOOT16	1
#else
#   error "TEXT_BASE must be 0xff000000"
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
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
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"bootfile=/tftpboot/MPC5200/uImage\0"				\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * IPB Bus clocking configuration.
 */
#undef CFG_IPBCLK_EQUALS_XLBCLK   		/* define for 133MHz speed */

/*
 * I2C configuration
 */
/*
 * EEPROM configuration
 */
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	70

#define CFG_I2C_EEPROM_ADDR_LEN 2
#define CFG_EEPROM_SIZE 0x2000

#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R

#undef	CONFIG_HARD_I2C			/* I2C with hardware support */
#define	CONFIG_SOFT_I2C		1	/* I2C with softwate support */

#if defined (CONFIG_SOFT_I2C)
#  define SDA0			0x40
#  define SCL0			0x80
#  define GPIOE0		*((volatile uchar*)(CFG_MBAR+0x0c00))
#  define DDR0			*((volatile uchar*)(CFG_MBAR+0x0c08))
#  define DVO0			*((volatile uchar*)(CFG_MBAR+0x0c0c))
#  define DVI0			*((volatile uchar*)(CFG_MBAR+0x0c20))
#  define ODE0			*((volatile uchar*)(CFG_MBAR+0x0c04))
#  define I2C_INIT		{GPIOE0|=(SDA0|SCL0);ODE0|=(SDA0|SCL0);DVO0|=(SDA0|SCL0);DDR0|=(SDA0|SCL0);}
#  define I2C_READ		((DVI0&SDA0)?1:0)
#  define I2C_SDA(x)	{if(x)DVO0|=SDA0;else DVO0&=~SDA0;}
#  define I2C_SCL(x)	{if(x)DVO0|=SCL0;else DVO0&=~SCL0;}
#  define I2C_DELAY		{udelay(5);}
#  define I2C_ACTIVE	{DDR0|=SDA0;}
#  define I2C_TRISTATE	{DDR0&=~SDA0;}
#  define CFG_I2C_SPEED		100000
#  define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_EEPROM_ADDR 0x57
#define CFG_I2C_FACT_ADDR	0x57
#endif

#if defined (CONFIG_HARD_I2C)
#  define CFG_I2C_MODULE	2		/* Select I2C module #1 or #2 */
#  define CFG_I2C_SPEED		100000	/* 100 kHz */
#  define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_EEPROM_ADDR 0x54
#define CFG_I2C_FACT_ADDR	0x54
#endif

/*
 * Flash configuration, expect one 16 Megabyte Bank at most
 */
#define CFG_FLASH_BASE		0xff000000
#define CFG_FLASH_SIZE		0x01000000
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks      */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0)

#define CFG_MAX_FLASH_SECT	256	/* max num of sects on one chip */

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

#undef CONFIG_FLASH_16BIT	/* Flash is 8-bit */

/*
 * DRAM configuration - will be read from VPD later... TODO!
 */
#if 0
/* 2x MT48LC16M16A2 - 7.0 ns SDRAMS = 64 MegaBytes Total */
#define	CFG_DRAM_DDR		0
#define CFG_DRAM_EMODE		0
#define CFG_DRAM_MODE		0x008D
#define CFG_DRAM_CONTROL	0x514F0000
#define CFG_DRAM_CONFIG1	0xC2233A00
#define CFG_DRAM_CONFIG2	0x88B70004
#define	CFG_DRAM_TAP_DEL	0x08
#define CFG_DRAM_RAM_SIZE	0x19
#endif
#if 1
/* 2x MT48LC16M16A2 - 7.5 ns SDRAMS = 64 MegaBytes Total */
#define	CFG_DRAM_DDR		0
#define CFG_DRAM_EMODE		0
#define CFG_DRAM_MODE		0x00CD
#define CFG_DRAM_CONTROL	0x514F0000
#define CFG_DRAM_CONFIG1	0xD2333A00
#define CFG_DRAM_CONFIG2	0x8AD70004
#define	CFG_DRAM_TAP_DEL	0x08
#define CFG_DRAM_RAM_SIZE	0x19
#endif

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_EEPROM	1	/* turn on EEPROM env feature */
#define CFG_ENV_OFFSET		0x1000
#define CFG_ENV_SIZE		0x0700

/*
 * VPD settings
 */
#define CFG_FACT_OFFSET		0x1800
#define CFG_FACT_SIZE		0x0800

/*
 * Memory map
 *
 * Warning!!! with the current BestComm Task, MBAR MUST BE set to 0xf0000000
 */
#define CFG_MBAR			0xf0000000	/* DO NOT CHANGE this */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE	/* End of used area in DPRAM */


#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_FEC_10MBIT	1		/* Workaround for FEC 100Mbit problem */
#define	CONFIG_PHY_ADDR		0x1f
#define	CONFIG_PHY_TYPE		0x79c874
/*
 * GPIO configuration:
 * PSC1,2,3 predefined as UART
 * PCI disabled
 * Ethernet 100 with MD
 */
#define CFG_GPS_PORT_CONFIG	0x00058044

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#  define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x01f00000	/* 1 ... 31 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x200000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif


#ifdef CONFIG_EVAL5200		/* M48T08 is available with the Evaluation board only */
  #define CONFIG_RTC_MK48T59	1	/* use M48T08 on EVAL5200 */
  #define RTC(reg)		(0xf0010000+reg)
  /* setup CS2 for M48T08. Must MAP 64kB */
  #define CFG_CS2_START	RTC(0)
  #define CFG_CS2_SIZE	0x10000
  /* setup CS2 configuration register: */
  /* WaitP = 0, WaitX = 4, MX=0, AL=1, AA=1, CE=1 */
  /* AS=2, DS=0, Bank=0, WTyp=0, WS=0, RS=0, WO=0, RO=0 */
  #define CFG_CS2_CFG	0x00047800
#else
  #define CONFIG_RTC_MPC5200	1	/* use internal MPC5200 RTC */
#endif

/*
 * Various low-level settings
 */
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00047801
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

#define CFG_RESET_ADDRESS	0x7f000000

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define CONFIG_IDE_RESET	1
#define CONFIG_IDE_PREINIT

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(CFG_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	(0x005c)

/* Interval between registers                                                */
#define CFG_ATA_STRIDE          4

#endif /* __CONFIG_H */
