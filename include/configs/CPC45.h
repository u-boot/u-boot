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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 *
 * Configuration settings for the CPC45 board.
 *
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
#define CONFIG_MPC8245		1
#define CONFIG_CPC45		1

#define CONFIG_SYS_TEXT_BASE	0xFFF00000

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#define CONFIG_BOOTDELAY	5

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH

#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/

#if 1
#define CONFIG_SYS_HUSH_PARSER		1	/* use "hush" command parser	*/
#endif
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

/* Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR	0x00100000	/* Default load address		*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */

#define CONFIG_SYS_SDRAM_BASE		0x00000000

#if defined(CONFIG_BOOT_ROM)
#define CONFIG_SYS_FLASH_BASE		0xFF000000
#else
#define CONFIG_SYS_FLASH_BASE		0xFF800000
#endif

#define CONFIG_SYS_RESET_ADDRESS	0xFFF00100

#define CONFIG_SYS_EUMB_ADDR		0xFCE00000

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

#define CONFIG_SYS_MEMTEST_START	0x00004000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x02000000	/* 0 ... 32 MB in DRAM		*/

/* Maximum amount of RAM.
 */
#define CONFIG_SYS_MAX_RAM_SIZE	0x10000000


#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
#undef CONFIG_SYS_RAMBOOT
#else
#define CONFIG_SYS_RAMBOOT
#endif


/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */

/* Size in bytes reserved for initial data
 */

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_EUMB_ADDR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_EUMB_ADDR + 0x4600)
#define DUART_DCR		(CONFIG_SYS_EUMB_ADDR + 0x4511)

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */

#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * RTC configuration
 */
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR	0x51

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x58
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 */

#define CONFIG_SYS_CLK_FREQ	33000000
#define CONFIG_SYS_HZ			1000


/* Bit-field values for MCCR1.
 */
#define CONFIG_SYS_ROMNAL		0
#define CONFIG_SYS_ROMFAL		8

#define CONFIG_SYS_BANK0_ROW		0	/* SDRAM bank 7-0 row address */
#define CONFIG_SYS_BANK1_ROW		0
#define CONFIG_SYS_BANK2_ROW		0
#define CONFIG_SYS_BANK3_ROW		0
#define CONFIG_SYS_BANK4_ROW		0
#define CONFIG_SYS_BANK5_ROW		0
#define CONFIG_SYS_BANK6_ROW		0
#define CONFIG_SYS_BANK7_ROW		0

/* Bit-field values for MCCR2.
 */

#define CONFIG_SYS_REFINT		0x2ec

/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.
 */
#define CONFIG_SYS_BSTOPRE		160

/* Bit-field values for MCCR3.
 */
#define CONFIG_SYS_REFREC		2	/* Refresh to activate interval		*/
#define CONFIG_SYS_RDLAT		0	/* Data latancy from read command	*/

/* Bit-field values for MCCR4.
 */
#define CONFIG_SYS_PRETOACT		2	/* Precharge to activate interval	*/
#define CONFIG_SYS_ACTTOPRE		5	/* Activate to Precharge interval	*/
#define CONFIG_SYS_SDMODE_CAS_LAT	2	/* SDMODE CAS latancy			*/
#define CONFIG_SYS_SDMODE_WRAP		0	/* SDMODE wrap type			*/
#define CONFIG_SYS_SDMODE_BURSTLEN	2	/* SDMODE Burst length			*/
#define CONFIG_SYS_ACTORW		2
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER 1
#define CONFIG_SYS_EXTROM		0
#define CONFIG_SYS_REGDIMM		0

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
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
#define CONFIG_SYS_BANK4_START		0x3ff00000
#define CONFIG_SYS_BANK4_END		0x3fffffff
#define CONFIG_SYS_BANK4_ENABLE	0
#define CONFIG_SYS_BANK5_START		0x3ff00000
#define CONFIG_SYS_BANK5_END		0x3fffffff
#define CONFIG_SYS_BANK5_ENABLE	0
#define CONFIG_SYS_BANK6_START		0x3ff00000
#define CONFIG_SYS_BANK6_END		0x3fffffff
#define CONFIG_SYS_BANK6_ENABLE	0
#define CONFIG_SYS_BANK7_START		0x3ff00000
#define CONFIG_SYS_BANK7_END		0x3fffffff
#define CONFIG_SYS_BANK7_ENABLE	0

#define CONFIG_SYS_ODCR		0xff
#define CONFIG_SYS_PGMAX		0x32	/* how long the 8240 retains the	*/
					/* currently accessed page in memory	*/
					/* see 8240 book for details		*/

#define CONFIG_SYS_IBAT0L  (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U  (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT1L  (CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U  (CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT2L  (0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U  (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT3L  (0xFC000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U  (0xFC000000 | BATU_BL_64M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT0L  CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U  CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT1L  CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U  CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT2L  CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U  CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT3L  CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U  CONFIG_SYS_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* Max number of flash banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	39	/* Max number of sectors in one bank	*/
#define INTEL_ID_28F160F3T	0x88F388F3	/*  16M = 1M x 16 top boot sector	*/
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

	/* Warining: environment is not EMBEDDED in the ppcboot code.
	 * It's stored in flash separately.
	 */
#define CONFIG_ENV_IS_IN_FLASH	    1

#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x7F8000)
#define CONFIG_ENV_SIZE		0x4000	/* Size of the Environment		*/
#define CONFIG_ENV_OFFSET		0	/* starting right at the beginning	*/
#define CONFIG_ENV_SECT_SIZE	0x8000 /* Size of the Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*----------------------------------------------------------------------*/
/* CPC45 Memory Map							*/
/*----------------------------------------------------------------------*/
#define SRAM_BASE	0x80000000	/* SRAM base address		*/
#define SRAM_END	0x801FFFFF
#define ST16552_A_BASE	0x80200000	/* ST16552 channel A		*/
#define ST16552_B_BASE	0x80400000	/* ST16552 channel A		*/
#define BCSR_BASE	0x80600000	/* board control / status registers */
#define DISPLAY_BASE	0x80600040	/* DISPLAY base			*/
#define PCMCIA_MEM_BASE 0x83000000	/* PCMCIA memory window base	*/
#define PCMCIA_IO_BASE	0xFE000000	/* PCMCIA IO window base	*/

#define	CONFIG_SYS_SRAM_BASE	SRAM_BASE
#define	CONFIG_SYS_SRAM_SIZE	(SRAM_END - SRAM_BASE + 1)

/*---------------------------------------------------------------------*/
/* CPC45 Control/Status Registers				       */
/*---------------------------------------------------------------------*/
#define IRQ_ENA_1		*((volatile uchar*)(BCSR_BASE + 0x00))
#define IRQ_STAT_1		*((volatile uchar*)(BCSR_BASE + 0x01))
#define IRQ_ENA_2		*((volatile uchar*)(BCSR_BASE + 0x02))
#define IRQ_STAT_2		*((volatile uchar*)(BCSR_BASE + 0x03))
#define BOARD_CTRL		*((volatile uchar*)(BCSR_BASE + 0x04))
#define BOARD_STAT		*((volatile uchar*)(BCSR_BASE + 0x05))
#define WDG_START		*((volatile uchar*)(BCSR_BASE + 0x06))
#define WDG_PRESTOP		*((volatile uchar*)(BCSR_BASE + 0x06))
#define WDG_STOP		*((volatile uchar*)(BCSR_BASE + 0x06))
#define BOARD_REV		*((volatile uchar*)(BCSR_BASE + 0x07))

/* IRQ_ENA_1 bit definitions */
#define I_ENA_1_IERA	0x80		/* INTA enable			*/
#define I_ENA_1_IERB	0x40		/* INTB enable			*/
#define I_ENA_1_IERC	0x20		/* INTC enable			*/
#define I_ENA_1_IERD	0x10		/* INTD enable			*/

/* IRQ_STAT_1 bit definitions */
#define I_STAT_1_INTA	0x80		/* INTA status			*/
#define I_STAT_1_INTB	0x40		/* INTB status			*/
#define I_STAT_1_INTC	0x20		/* INTC status			*/
#define I_STAT_1_INTD	0x10		/* INTD status			*/

/* IRQ_ENA_2 bit definitions */
#define I_ENA_2_IEAB	0x80		/* ABORT IRQ enable		*/
#define I_ENA_2_IEK1	0x40		/* KEY1 IRQ enable		*/
#define I_ENA_2_IEK2	0x20		/* KEY2 IRQ enable		*/
#define I_ENA_2_IERT	0x10		/* RTC IRQ enable		*/
#define I_ENA_2_IESM	0x08		/* LM81 IRQ enable		*/
#define I_ENA_2_IEDG	0x04		/* DEGENERATING IRQ enable	*/
#define I_ENA_2_IES2	0x02		/* ST16552/B IRQ enable		*/
#define I_ENA_2_IES1	0x01		/* ST16552/A IRQ enable		*/

/* IRQ_STAT_2 bit definitions */
#define I_STAT_2_ABO	0x80		/* ABORT IRQ status		*/
#define I_STAT_2_KY1	0x40		/* KEY1 IRQ status		*/
#define I_STAT_2_KY2	0x20		/* KEY2 IRQ status		*/
#define I_STAT_2_RTC	0x10		/* RTC IRQ status		*/
#define I_STAT_2_SMN	0x08		/* LM81 IRQ status		*/
#define I_STAT_2_DEG	0x04		/* DEGENERATING IRQ status	*/
#define I_STAT_2_SIO2	0x02		/* ST16552/B IRQ status		*/
#define I_STAT_2_SIO1	0x01		/* ST16552/A IRQ status		*/

/* BOARD_CTRL bit definitions */
#define USER_LEDS		2			/* 2 user LEDs	*/

#if (USER_LEDS == 4)
#define B_CTRL_WRSE		0x80
#define B_CTRL_KRSE		0x40
#define B_CTRL_FWRE		0x20		/* Flash write enable		*/
#define B_CTRL_FWPT		0x10		/* Flash write protect		*/
#define B_CTRL_LED3		0x08		/* LED 3 control		*/
#define B_CTRL_LED2		0x04		/* LED 2 control		*/
#define B_CTRL_LED1		0x02		/* LED 1 control		*/
#define B_CTRL_LED0		0x01		/* LED 0 control		*/
#else
#define B_CTRL_WRSE		0x80
#define B_CTRL_KRSE		0x40
#define B_CTRL_FWRE_1		0x20		/* Flash write enable		*/
#define B_CTRL_FWPT_1		0x10		/* Flash write protect		*/
#define B_CTRL_LED1		0x08		/* LED 1 control		*/
#define B_CTRL_LED0		0x04		/* LED 0 control		*/
#define B_CTRL_FWRE_0		0x02		/* Flash write enable		*/
#define B_CTRL_FWPT_0		0x01		/* Flash write protect		*/
#endif

/* BOARD_STAT bit definitions */
#define B_STAT_WDGE		0x80
#define B_STAT_WDGS		0x40
#define B_STAT_WRST		0x20
#define B_STAT_KRST		0x10
#define B_STAT_CSW3		0x08		/* sitch bit 3 status		*/
#define B_STAT_CSW2		0x04		/* sitch bit 2 status		*/
#define B_STAT_CSW1		0x02		/* sitch bit 1 status		*/
#define B_STAT_CSW0		0x01		/* sitch bit 0 status		*/

/*---------------------------------------------------------------------*/
/* Display addresses						       */
/*---------------------------------------------------------------------*/
#define DISP_UDC_RAM	(DISPLAY_BASE + 0x08)	/* UDC RAM	       */
#define DISP_CHR_RAM	(DISPLAY_BASE + 0x18)	/* character Ram       */
#define DISP_FLASH	(DISPLAY_BASE + 0x20)	/* Flash Ram	       */

#define DISP_UDC_ADR	*((volatile uchar*)(DISPLAY_BASE + 0x00))	/* UDC Address Reg.    */
#define DISP_CWORD	*((volatile uchar*)(DISPLAY_BASE + 0x10))	/* Control Word Reg.   */

#define DISP_DIG0	*((volatile uchar*)(DISP_CHR_RAM + 0x00))	/* Digit 0 address     */
#define DISP_DIG1	*((volatile uchar*)(DISP_CHR_RAM + 0x01))	/* Digit 0 address     */
#define DISP_DIG2	*((volatile uchar*)(DISP_CHR_RAM + 0x02))	/* Digit 0 address     */
#define DISP_DIG3	*((volatile uchar*)(DISP_CHR_RAM + 0x03))	/* Digit 0 address     */
#define DISP_DIG4	*((volatile uchar*)(DISP_CHR_RAM + 0x04))	/* Digit 0 address     */
#define DISP_DIG5	*((volatile uchar*)(DISP_CHR_RAM + 0x05))	/* Digit 0 address     */
#define DISP_DIG6	*((volatile uchar*)(DISP_CHR_RAM + 0x06))	/* Digit 0 address     */
#define DISP_DIG7	*((volatile uchar*)(DISP_CHR_RAM + 0x07))	/* Digit 0 address     */


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_PCI			/* include pci support			*/
#define CONFIG_SYS_EARLY_PCI_INIT
#undef	CONFIG_PCI_PNP
#undef	CONFIG_PCI_SCAN_SHOW

#define CONFIG_NET_MULTI		/* Multi ethernet cards support		*/

#define CONFIG_EEPRO100
#define CONFIG_SYS_RX_ETH_BUFFER	8	/* use 8 rx buffer on eepro100	*/

#define PCI_ENET0_IOADDR	0x82000000
#define PCI_ENET0_MEMADDR	0x82000000
#define PCI_PLX9030_IOADDR	0x82100000
#define PCI_PLX9030_MEMADDR	0x82100000

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 */

#define CONFIG_I82365

#define CONFIG_SYS_PCMCIA_MEM_ADDR	PCMCIA_MEM_BASE
#define CONFIG_SYS_PCMCIA_MEM_SIZE	0x1000

#define CONFIG_PCMCIA_SLOT_A

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define	CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for IDE not supported	*/
#define	CONFIG_IDE_LED			/* LED   for IDE is  supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_PCMCIA_MEM_ADDR

#define CONFIG_SYS_ATA_DATA_OFFSET	CONFIG_SYS_PCMCIA_MEM_SIZE

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	(CONFIG_SYS_PCMCIA_MEM_SIZE + 0x400)

#define CONFIG_DOS_PARTITION

#endif	/* __CONFIG_H */
