/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2005-2007
 * Beijing UD Technology Co., Ltd., taihusupport@amcc.com
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


#define CONFIG_405EP		1	/* this is a PPC405 CPU */
#define CONFIG_4xx		1	/*  member of PPC4xx family */
#define CONFIG_TAIHU	        1	/*  on a taihu board */

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f */

#define CONFIG_SYS_CLK_FREQ     33000000 /* external frequency to pll   */

#define CONFIG_NO_SERIAL_EEPROM

/*----------------------------------------------------------------------------*/
#ifdef CONFIG_NO_SERIAL_EEPROM

/*
!-------------------------------------------------------------------------------
! PLL settings for 333MHz CPU, 111MHz PLB/SDRAM, 55MHz EBC, 33MHz PCI,
! assuming a 33MHz input clock to the 405EP from the C9531.
!-------------------------------------------------------------------------------
*/
#define PLLMR0_333_111_55_37 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_3)
#define PLLMR1_333_111_55_37 (PLL_FBKDIV_10  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)
#define PLLMR0_333_111_55_111 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			       PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			       PLL_MALDIV_1 | PLL_PCIDIV_1)
#define PLLMR1_333_111_55_111 (PLL_FBKDIV_10  |  \
			       PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			       PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)

#define PLLMR0_DEFAULT		PLLMR0_333_111_55_37
#define PLLMR1_DEFAULT		PLLMR1_333_111_55_37
#define PLLMR0_DEFAULT_PCI66	PLLMR0_333_111_55_111
#define PLLMR1_DEFAULT_PCI66	PLLMR1_333_111_55_111

#endif
/*----------------------------------------------------------------------------*/

#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars */

#define CONFIG_ENV_OVERWRITE 1
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootfile=/tftpboot/taihu/uImage\0"				\
	"rootpath=/opt/eldk/ppc_4xx\0"					\
	"netdev=eth0\0"							\
	"hostname=taihu\0"						\
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
	"kernel_addr=FC000000\0"					\
	"ramdisk_addr=FC180000\0"					\
	"load=tftp 200000 /tftpboot/taihu/u-boot.bin\0"			\
	"update=protect off FFFC0000 FFFFFFFF;era FFFC0000 FFFFFFFF;"	\
		"cp.b 200000 FFFC0000 40000\0"				\
	"upd=run load update\0"						\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0x14	/* PHY address			*/
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	0x10	/* EMAC1 PHY address		*/
#define CONFIG_NET_MULTI	1
#define CFG_RX_ETH_BUFFER	16	/* Number of ethernet rx buffers & descriptors */
#define CONFIG_PHY_RESET	1

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

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SPI

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#undef CONFIG_SPD_EEPROM		/* use SPD EEPROM for setup */
#define CFG_SDRAM_SIZE_PER_BANK 0x04000000 /* 64MB */
#define CFG_SDRAM_BANKS	        2

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

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* print buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START  0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END	   0x0C00000	/* 4 ... 12 MB in DRAM	*/

/*
 * If CFG_EXT_SERIAL_CLOCK, then the UART divisor is 1.
 * If CFG_405_UART_ERRATA_59, then UART divisor is 31.
 * Otherwise, UART divisor is determined by CPU Clock and CFG_BASE_BAUD value.
 * The Linux BASE_BAUD define should match this configuration.
 *    baseBaud = cpuClock/(uartDivisor*16)
 * If CFG_405_UART_ERRATA_59 and 200MHz CPU clock,
 * set Linux BASE_BAUD to 403200.
 */
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#undef  CFG_EXT_SERIAL_CLOCK           /* external serial clock */
#undef  CFG_405_UART_ERRATA_59         /* 405GP/CR Rev. D silicon */
#define CFG_BASE_BAUD		691200

#define CONFIG_BAUDRATE		115200

#define CONFIG_UART1_CONSOLE	1

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CFG_LOAD_ADDR	    0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */
#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

/*-----------------------------------------------------------------------
 * I2C stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef  CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_NOPROBES	{ 0x69 } /* avoid iprobe hangup (why?) */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	6 /* 24C02 requires 5ms delay */

#define CFG_I2C_EEPROM_ADDR	0x50	/* I2C boot EEPROM (24C02W)	*/
#define CFG_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address		*/

#define CONFIG_SOFT_SPI
#define SPI_SCL  spi_scl
#define SPI_SDA  spi_sda
#define SPI_READ spi_read()
#define SPI_DELAY udelay(2)
#ifndef __ASSEMBLY__
void spi_scl(int);
void spi_sda(int);
unsigned char spi_read(void);
#endif

/* standard dtt sensor configuration */
#define CONFIG_DTT_DS1775	1
#define CONFIG_DTT_SENSORS	{ 0 }
#define CFG_I2C_DTT_ADDR	0x49

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter    */
#define PCI_HOST_FORCE   1		/* configure as pci host       */
#define PCI_HOST_AUTO    2		/* detected via arbiter enable */

#define CONFIG_PCI			/* include pci support	       */
#define CONFIG_PCI_HOST	PCI_HOST_FORCE  /* select pci host function    */
#define CONFIG_PCI_PNP			/* do pci plug-and-play        */
					/* resource configuration      */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup */

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CFG_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */
#define CFG_PCI_CLASSCODE       0x0600  /* PCI Class Code: bridge/host */
#define CFG_PCI_PTM1LA	    0x00000000	/* point to sdram              */
#define CFG_PCI_PTM1MS      0x80000001	/* 2GB, enable hard-wired to 1 */
#define CFG_PCI_PTM1PCI     0x00000000	/* Host: use this pci address  */
#define CFG_PCI_PTM2LA      0x00000000	/* disabled                    */
#define CFG_PCI_PTM2MS	    0x00000000	/* disabled                    */
#define CFG_PCI_PTM2PCI     0x04000000	/* Host: use this pci address  */
#define CONFIG_EEPRO100		1

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFE00000
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

#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_ADDR0         0x555
#define CFG_FLASH_ADDR1         0x2aa
#define CFG_FLASH_WORD_SIZE     unsigned short

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000	/* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CFG_NVRAM_BASE_ADDR	0xf0000000	/* NVRAM base address */
#define CFG_NVRAM_SIZE		0x1ff8		/* NVRAM size */

#ifdef CFG_ENV_IS_IN_NVRAM
#define CFG_ENV_SIZE		0x0ff8		/* Size of Environment vars */
#define CFG_ENV_ADDR		\
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_ENV_SIZE)	/* Env*/
#endif

/*-----------------------------------------------------------------------
 * PPC405 GPIO Configuration
 */
#define CFG_4xx_GPIO_TABLE { /*				GPIO	Alternate1		*/	\
{												\
/* GPIO Core 0 */										\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO0	PerBLast    SPI CS	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO1	TS1E			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO2	TS2E			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO3	TS1O			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO4	TS2O			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO5	TS3			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO6	TS4			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO7	TS5			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO8	TS6			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO9	TrcClk			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO10	PerCS1			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO11	PerCS2			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO12	PerCS3			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO13	PerCS4			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO14	PerAddr03   SPI SCLK	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO15	PerAddr04   SPI DI	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO16	PerAddr05   SPI DO	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO17	IRQ0	    PCI INTA	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO18	IRQ1	    PCI INTB	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO19	IRQ2	    PCI INTC	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO20	IRQ3	    PCI INTD	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO21	IRQ4	    USB		*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO22	IRQ5	    EBC		*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO23	IRQ6	    unused	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO24	UART0_DCD   UART1	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO25	UART0_DSR		*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO26	UART0_RI		*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO27	UART0_DTR		*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO28	UART1_Rx    UART0 	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO29	UART1_Tx		*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO30	RejectPkt0  User LED1	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO31	RejectPkt1  User LED2	*/	\
}												\
}

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM CFG_FLASH_BASE /* FLASH bank #0 */
#define FLASH_BASE1_PRELIM  0xFC000000	/* FLASH bank #1 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory (OCM) for temperary stack until sdram is tested */
#define CFG_TEMP_STACK_OCM        1

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/

#define CFG_GBL_DATA_SIZE      128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash/SRAM) initialization */
#define CFG_EBC_PB0AP           0x03815600
#define CFG_EBC_PB0CR           0xFFE3A000  /* BAS=0xFFE,BS=2MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (NVRAM/RTC) initialization */
#define CFG_EBC_PB1AP           0x05815600
#define CFG_EBC_PB1CR           0xFC0BA000  /* BAS=0xFc0,BS=32MB,BU=R/W,BW=16bit */

/* Memory Bank 2 (USB device) initialization */
#define CFG_EBC_PB2AP           0x03016600
#define CFG_EBC_PB2CR           0x50018000 /* BAS=0x500,BS=1MB,BU=R/W,BW=8bit */

/* Memory Bank 3 (LCM and D-flip-flop) initialization */
#define CFG_EBC_PB3AP           0x158FF600
#define CFG_EBC_PB3CR           0x50118000 /* BAS=0x501,BS=1MB,BU=R/W,BW=8bit */

/* Memory Bank 4 (not install) initialization */
#define CFG_EBC_PB4AP           0x158FF600
#define CFG_EBC_PB4CR           0x5021A000

#define CPLD_REG0_ADDR	0x50100000
#define CPLD_REG1_ADDR	0x50100001
/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

#endif	/* __CONFIG_H */
