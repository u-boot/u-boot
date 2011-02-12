/*
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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

#include <asm/ibmpc.h>
/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Stuff still to be dealt with -
 */
#define CONFIG_RTC_MC146818
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS	0

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define DEBUG_PARSER

#define CONFIG_X86			1	/* Intel X86 CPU */
#define CONFIG_SYS_SC520		1	/* AMD SC520 */
#define CONFIG_SYS_SC520_SSI
#define CONFIG_SHOW_BOOT_PROGRESS	1
#define CONFIG_LAST_STAGE_INIT		1

/*
 * If CONFIG_HW_WATCHDOG is not defined, the watchdog jumper on the
 * bottom (processor) board MUST be removed!
 */
#undef CONFIG_WATCHDOG
#define CONFIG_HW_WATCHDOG

 /*-----------------------------------------------------------------------
  * Serial Configuration
  */
#define CONFIG_SERIAL_MULTI
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		1843200
#define CONFIG_BAUDRATE			9600
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1		UART0_BASE
#define CONFIG_SYS_NS16550_COM2		UART1_BASE
#define CONFIG_SYS_NS16550_COM3		(0x1000 + UART0_BASE)
#define CONFIG_SYS_NS16550_COM4		(0x1000 + UART1_BASE)
#define CONFIG_SYS_NS16550_PORT_MAPPED

 /*-----------------------------------------------------------------------
  * Video Configuration
  */
#undef CONFIG_VIDEO			/* No Video Hardware */
#undef CONFIG_CFB_CONSOLE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)

/*-----------------------------------------------------------------------
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BDI		/* bdinfo			*/
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_FLASH	/* flinfo, erase, protect	*/
#define CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#define CONFIG_CMD_IMI		/* iminfo			*/
#define CONFIG_CMD_IMLS		/* List all found images	*/
#define CONFIG_CMD_IRQ		/* IRQ Information		*/
#define CONFIG_CMD_ITEST	/* Integer (and string) test	*/
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#undef CONFIG_CMD_NFS		/* NFS support			*/
#define CONFIG_CMD_PCI		/* PCI support			*/
#define CONFIG_CMD_PING		/* ICMP echo support		*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command Support	*/
#define CONFIG_CMD_XIMG		/* Load part of Multi Image	*/

#define CONFIG_BOOTDELAY		15
#define CONFIG_BOOTARGS			"root=/dev/mtdblock0 console=ttyS0,9600"
/* #define CONFIG_BOOTCOMMAND		"bootm 38000000" */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE		115200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX		2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"boot > "	/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + \
					 16)		/* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x01000000	/* 1 ... 16 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ			1000		/* incrementer freq: 1kHz */

/*-----------------------------------------------------------------------
 * SDRAM Configuration
 */
#define CONFIG_SYS_SDRAM_DRCTMCTL	0x18
#define CONFIG_NR_DRAM_BANKS		4

/* CONFIG_SYS_SDRAM_DRCTMCTL Overrides the following*/
#undef CONFIG_SYS_SDRAM_PRECHARGE_DELAY
#undef CONFIG_SYS_SDRAM_REFRESH_RATE
#undef CONFIG_SYS_SDRAM_RAS_CAS_DELAY
#undef CONFIG_SYS_SDRAM_CAS_LATENCY_2T
#undef CONFIG_SYS_SDRAM_CAS_LATENCY_3T

/*-----------------------------------------------------------------------
 * CPU Features
 */
#define CONFIG_SYS_SC520_HIGH_SPEED	0	/* 100 or 133MHz */
#define CONFIG_SYS_SC520_RESET			/* use SC520 MMCR's to reset cpu */
#define CONFIG_SYS_SC520_TIMER			/* use SC520 swtimers */
#undef  CONFIG_SYS_GENERIC_TIMER		/* use the i8254 PIT timers */
#undef  CONFIG_SYS_TSC_TIMER			/* use the Pentium TSC timers */
#define CONFIG_SYS_USE_SIO_UART		0       /* prefer the uarts on the SIO to those
					 * in the SC520 on the CDP */
#define CONFIG_SYS_PCAT_INTERRUPTS
#define CONFIG_SYS_NUM_IRQS		16

/*-----------------------------------------------------------------------
 * Memory organization
 */
#define CONFIG_SYS_STACK_SIZE		0x8000  	/* Size of bootloader stack */
#define CONFIG_SYS_BL_START_FLASH	0x38040000	/* Address of relocated code */
#define CONFIG_SYS_BL_START_RAM		0x03fd0000	/* Address of relocated code */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)    /* Reserve 256 kB for Mon	*/
#define CONFIG_SYS_FLASH_BASE		0x38000000	/* Boot Flash */
#define CONFIG_SYS_FLASH_BASE_1		0x10000000	/* StrataFlash 1 */
#define CONFIG_SYS_FLASH_BASE_2		0x11000000	/* StrataFlash 2 */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

 /*-----------------------------------------------------------------------
  * FLASH configuration
  */
#define CONFIG_FLASH_CFI_DRIVER				/* Use the common driver */
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_FLASH_CFI				/* Flash is CFI conformant */
#define CONFIG_SYS_MAX_FLASH_BANKS	3	/* max number of memory banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE,   \
					 CONFIG_SYS_FLASH_BASE_1, \
					 CONFIG_SYS_FLASH_BASE_2}
#define CONFIG_SYS_FLASH_EMPTY_INFO
#undef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max number of sectors on one chip */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CONFIG_SYS_FLASH_LEGACY_512Kx8

 /*-----------------------------------------------------------------------
  * Environment configuration
  */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SECT_SIZE		0x20000 /* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR			CONFIG_SYS_FLASH_BASE_1
/* Redundant Copy */
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_SYS_FLASH_BASE_1 + \
					 CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SECT_SIZE


 /*-----------------------------------------------------------------------
  * PCI configuration
  */
#define CONFIG_PCI                                /* include pci support */
#define CONFIG_PCI_PNP                            /* pci plug-and-play */
#define CONFIG_SYS_FIRST_PCI_IRQ   10
#define CONFIG_SYS_SECOND_PCI_IRQ  9
#define CONFIG_SYS_THIRD_PCI_IRQ   11
#define CONFIG_SYS_FORTH_PCI_IRQ   15

 /*
 * Network device (TRL8100B) support
 */
#define CONFIG_NET_MULTI
#define CONFIG_RTL8139

/*-----------------------------------------------------------------------
 * FPGA configuration
 */
#define CONFIG_SYS_FPGA_PROGRAM_PIO_BIT		0x2000
#define CONFIG_SYS_FPGA_INIT_PIO_BIT		0x4000
#define CONFIG_SYS_FPGA_DONE_PIO_BIT		0x8000
#define CONFIG_SYS_FPGA_PIO_DATA 		SC520_PIODATA31_16
#define CONFIG_SYS_FPGA_PIO_DIRECTION 		SC520_PIODIR31_16
#define CONFIG_SYS_FPGA_PIO_CLR  		SC520_PIOCLR31_16
#define CONFIG_SYS_FPGA_PIO_SET  		SC520_PIOSET31_16
#define CONFIG_SYS_FPGA_PROGRAM_BIT_DROP_TIME	1	/* milliseconds */
#define CONFIG_SYS_FPGA_MAX_INIT_TIME		10	/* milliseconds */
#define CONFIG_SYS_FPGA_MAX_FINALISE_TIME	10	/* milliseconds */
#define CONFIG_SYS_FPGA_SSI_DATA_RATE		8333	/* kHz (33.3333MHz xtal) */

/*-----------------------------------------------------------------------
 * BOOTCS Control (for AM29LV040B-120JC)
 *
 * 000 0 00 0 000 11 0 011 }- 0x0033
 * \ / | \| | \ / \| | \ /
 *  |  |  | |  |   | |  |
 *  |  |  | |  |   | |  +---- 3 Wait States (First Access)
 *  |  |  | |  |   | +------- Reserved
 *  |  |  | |  |   +--------- 3 Wait States (Subsequent Access)
 *  |  |  | |  +------------- Reserved
 *  |  |  | +---------------- Non-Paged Mode
 *  |  |  +------------------ 8 Bit Wide
 *  |  +--------------------- GP Bus
 *  +------------------------ Reserved
 */
#define CONFIG_SYS_SC520_BOOTCS_CTRL		0x0033

/*-----------------------------------------------------------------------
 * ROMCS Control (for E28F128J3A-150 StrataFlash)
 *
 * 000 0 01 1 000 01 0 101 }- 0x0615
 * \ / | \| | \ / \| | \ /
 *  |  |  | |  |   | |  |
 *  |  |  | |  |   | |  +---- 5 Wait States (First Access)
 *  |  |  | |  |   | +------- Reserved
 *  |  |  | |  |   +--------- 1 Wait State (Subsequent Access)
 *  |  |  | |  +------------- Reserved
 *  |  |  | +---------------- Paged Mode
 *  |  |  +------------------ 16 Bit Wide
 *  |  +--------------------- GP Bus
 *  +------------------------ Reserved
 */
#define CONFIG_SYS_SC520_ROMCS1_CTRL		0x0615
#define CONFIG_SYS_SC520_ROMCS2_CTRL		0x0615

/*-----------------------------------------------------------------------
 * SC520 General Purpose Bus configuration
 *
 * Chip Select Offset		1 Clock Cycle
 * Chip Select Pulse Width	8 Clock Cycles
 * Chip Select Read Offset	2 Clock Cycles
 * Chip Select Read Width	6 Clock Cycles
 * Chip Select Write Offset	2 Clock Cycles
 * Chip Select Write Width	6 Clock Cycles
 * Chip Select Recovery Time	2 Clock Cycles
 *
 * Timing Diagram (from SC520 Register Set Manual - Order #22005B)
 *
 *   |<-------------General Purpose Bus Cycle---------------->|
 *   |                                                        |
 * ----------------------\__________________/------------------
 *   |<--(GPCSOFF + 1)-->|<--(GPCSPW + 1)-->|<-(GPCSRT + 1)-> |
 *
 * ------------------------\_______________/-------------------
 *   |<---(GPRDOFF + 1)--->|<-(GPRDW + 1)->|
 *
 * --------------------------\_______________/-----------------
 *   |<----(GPWROFF + 1)---->|<-(GPWRW + 1)->|
 *
 * ________/-----------\_______________________________________
 *   |<--->|<--------->|
 *      ^         ^
 * (GPALEOFF + 1) |
 *                |
 *         (GPALEW + 1)
 */
#define CONFIG_SYS_SC520_GPCSOFF		0x00
#define CONFIG_SYS_SC520_GPCSPW			0x07
#define CONFIG_SYS_SC520_GPRDOFF		0x01
#define CONFIG_SYS_SC520_GPRDW			0x05
#define CONFIG_SYS_SC520_GPWROFF		0x01
#define CONFIG_SYS_SC520_GPWRW			0x05
#define CONFIG_SYS_SC520_GPCSRT			0x01

/*-----------------------------------------------------------------------
 * SC520 Programmable I/O configuration
 *
 * Pin	  Mode		Dir.	Description
 * ----------------------------------------------------------------------
 * PIO0   PIO		Output	Unused
 * PIO1   GPBHE#	Output	GP Bus Byte High Enable (active low)
 * PIO2   PIO		Output	Auxiliary power output enable
 * PIO3   GPAEN		Output	GP Bus Address Enable
 * PIO4   PIO		Output	Top Board Enable (active low)
 * PIO5   PIO		Output	StrataFlash 16 bit mode (low = 8 bit mode)
 * PIO6   PIO		Input	Data output of Power Supply ADC
 * PIO7   PIO		Output	Clock input to Power Supply ADC
 * PIO8   PIO		Output  Chip Select input of Power Supply ADC
 * PIO9   PIO		Output	StrataFlash 1 Reset / Power Down (active low)
 * PIO10  PIO		Output	StrataFlash 2 Reset / Power Down (active low)
 * PIO11  PIO		Input	StrataFlash 1 Status
 * PIO12  PIO		Input	StrataFlash 2 Status
 * PIO13  GPIRQ10#	Input	Can Bus / I2C IRQ (active low)
 * PIO14  PIO		Input	Low Input Voltage Warning (active low)
 * PIO15  PIO		Output	Watchdog (must toggle at least every 1.6s)
 * PIO16  PIO		Input	Power Fail
 * PIO17  GPIRQ6	Input	Compact Flash 1 IRQ (active low)
 * PIO18  GPIRQ5	Input	Compact Flash 2 IRQ (active low)
 * PIO19  GPIRQ4#	Input	Dual-Port RAM IRQ (active low)
 * PIO20  GPIRQ3	Input	UART D IRQ
 * PIO21  GPIRQ2	Input	UART C IRQ
 * PIO22  GPIRQ1	Input	UART B IRQ
 * PIO23  GPIRQ0	Input	UART A IRQ
 * PIO24  GPDBUFOE#	Output	GP Bus Data Bus Buffer Output Enable
 * PIO25  PIO		Input	Battery OK Indication
 * PIO26  GPMEMCS16#	Input	GP Bus Memory Chip-Select 16-bit access
 * PIO27  GPCS0#	Output	SRAM 1 Chip Select
 * PIO28  PIO		Input	Top Board UART CTS
 * PIO29  PIO		Output	FPGA Program Mode (active low)
 * PIO30  PIO		Input	FPGA Initialised (active low)
 * PIO31  PIO		Input	FPGA Done (active low)
 */
#define CONFIG_SYS_SC520_PIOPFS15_0		0x200a
#define CONFIG_SYS_SC520_PIOPFS31_16		0x0dfe
#define CONFIG_SYS_SC520_PIODIR15_0		0x87bf
#define CONFIG_SYS_SC520_PIODIR31_16		0x2900

/*-----------------------------------------------------------------------
 * PIO Pin defines
 */
#define CONFIG_SYS_ENET_AUX_PWR			0x0004
#define CONFIG_SYS_ENET_TOP_BRD_PWR		0x0010
#define CONFIG_SYS_ENET_SF_WIDTH		0x0020
#define CONFIG_SYS_ENET_PWR_ADC_DATA		0x0040
#define CONFIG_SYS_ENET_PWR_ADC_CLK		0x0080
#define CONFIG_SYS_ENET_PWR_ADC_CS		0x0100
#define CONFIG_SYS_ENET_SF1_MODE		0x0200
#define CONFIG_SYS_ENET_SF2_MODE		0x0400
#define CONFIG_SYS_ENET_SF1_STATUS		0x0800
#define CONFIG_SYS_ENET_SF2_STATUS		0x1000
#define CONFIG_SYS_ENET_PWR_STATUS		0x4000
#define CONFIG_SYS_ENET_WATCHDOG		0x8000

#define CONFIG_SYS_ENET_PWR_FAIL		0x0001
#define CONFIG_SYS_ENET_BAT_OK			0x0200
#define CONFIG_SYS_ENET_TOP_BRD_CTS		0x1000
#define CONFIG_SYS_ENET_FPGA_PROG		0x2000
#define CONFIG_SYS_ENET_FPGA_INIT		0x4000
#define CONFIG_SYS_ENET_FPGA_DONE		0x8000

/*-----------------------------------------------------------------------
 * Chip Select Pin Function Select
 *
 * 1 1 1 1 1 0 0 0 }- 0xf8
 * | | | | | | | |
 * | | | | | | | +--- Reserved
 * | | | | | | +----- GPCS1_SEL = ROMCS1#
 * | | | | | +------- GPCS2_SEL = ROMCS2#
 * | | | | +--------- GPCS3_SEL = GPCS3
 * | | | +----------- GPCS4_SEL = GPCS4
 * | | +------------- GPCS5_SEL = GPCS5
 * | +--------------- GPCS6_SEL = GPCS6
 * +----------------- GPCS7_SEL = GPCS7
 */
#define CONFIG_SYS_SC520_CSPFS			0xf8

/*-----------------------------------------------------------------------
 * Clock Select (CLKTIMER[CLKTEST] pin)
 *
 * 0 111 00 1 0 }- 0x72
 * | \ / \| | |
 * |  |   | | +--- Pin Disabled
 * |  |   | +----- Pin is an output
 * |  |   +------- Reserved
 * |  +----------- Disabled (pin stays Low)
 * +-------------- Reserved
 */
#define CONFIG_SYS_SC520_CLKSEL			0x72

/*-----------------------------------------------------------------------
 * Address Decode Control
 *
 * 0 00 0 0 0 0 0 }- 0x00
 * | \| | | | | |
 * |  | | | | | +--- Integrated UART 1 is enabled
 * |  | | | | +----- Integrated UART 2 is enabled
 * |  | | | +------- Integrated RTC is enabled
 * |  | | +--------- Reserved
 * |  | +----------- I/O Hole accesses are forwarded to the external GP bus
 * |  +------------- Reserved
 * +---------------- Write-protect violations do not generate an IRQ
 */
#define CONFIG_SYS_SC520_ADDDECCTL		0x00

/*-----------------------------------------------------------------------
 * UART Control
 *
 * 00000 1 1 1 }- 0x07
 * \___/ | | |
 *   |   | | +--- Transmit TC interrupt enable
 *   |   | +----- Receive TC interrupt enable
 *   |   +------- 1.8432 MHz
 *   +----------- Reserved
 */
#define CONFIG_SYS_SC520_UART1CTL		0x07
#define CONFIG_SYS_SC520_UART2CTL		0x07

/*-----------------------------------------------------------------------
 * System Arbiter Control
 *
 * 00000 1 1 0 }- 0x06
 * \___/ | | |
 *   |   | | +--- Disable PCI Bus Arbiter Grant Time-Out Interrupt
 *   |   | +----- The system arbiter operates in concurrent mode
 *   |   +------- Park the PCI bus on the last master that acquired the bus
 *   +----------- Reserved
 */
#define CONFIG_SYS_SC520_SYSARBCTL		0x06

/*-----------------------------------------------------------------------
 * System Arbiter Master Enable
 *
 * 00000000000 0 0 0 1 1 }- 0x06
 * \_________/ | | | | |
 *      |      | | | | +--- PCI master REQ0 enabled (Ethernet 1)
 *      |      | | | +----- PCI master REQ1 enabled (Ethernet 2)
 *      |      | | +------- PCI master REQ2 disabled
 *      |      | +--------- PCI master REQ3 disabled
 *      |      +----------- PCI master REQ4 disabled
 *      +------------------ Reserved
 */
#define CONFIG_SYS_SC520_SYSARBMENB		0x0003

/*-----------------------------------------------------------------------
 * System Arbiter Master Enable
 *
 * 0 0000 0 00 0000 1 000 }- 0x06
 * | \__/ | \| \__/ | \_/
 * |   |  |  |   |  |  +---- Reserved
 * |   |  |  |   |  +------- Enable CPU-to-PCI bus write posting
 * |   |  |  |   +---------- Reserved
 * |   |  |  +-------------- PCI bus reads to SDRAM are not automatically
 * |   |  |                  retried
 * |   |  +----------------- Target read FIFOs are not snooped during write
 * |   |                     transactions
 * |   +-------------------- Reserved
 * +------------------------ Deassert the PCI bus reset signal
 */
#define CONFIG_SYS_SC520_HBCTL			0x08

/*-----------------------------------------------------------------------
 * PAR for Boot Flash - 512kB @ 0x38000000, BOOTCS
 * 100 0 1 0 1 00000000111 11100000000000 }- 0x8a01f800
 * \ / | | | | \----+----/ \-----+------/
 *  |  | | | |      |            +---------- Start at 0x38000000
 *  |  | | | |      +----------------------- 512kB Region Size
 *  |  | | | |                               ((7 + 1) * 64kB)
 *  |  | | | +------------------------------ 64kB Page Size
 *  |  | | +-------------------------------- Writes Enabled (So it can be
 *  |  | |                                   reprogrammed!)
 *  |  | +---------------------------------- Caching Disabled
 *  |  +------------------------------------ Execution Enabled
 *  +--------------------------------------- BOOTCS
 */
#define CONFIG_SYS_SC520_BOOTCS_PAR		0x8a01f800

/*-----------------------------------------------------------------------
 * PAR for Low Level I/O (LEDs, Hex Switches etc) - 33 Bytes @ 0x1000, GPCS6
 *
 * 001 110 0 000100000 0001000000000000 }- 0x38201000
 * \ / \ / | \---+---/ \------+-------/
 *  |   |  |     |            +----------- Start at 0x00001000
 *  |   |  |     +------------------------ 33 Bytes (0x20 + 1)
 *  |   |  +------------------------------ Ignored
 *  |   +--------------------------------- GPCS6
 *  +------------------------------------- GP Bus I/O
 */
#define CONFIG_SYS_SC520_LLIO_PAR		0x38201000

/*-----------------------------------------------------------------------
 * PAR for Compact Flash Port #1 - 4kB @ 0x200000000, CS5
 * PAR for Compact Flash Port #2 - 4kB @ 0x200010000, CS7
 *
 * 010 101 0 0000000 100000000000000000 }- 0x54020000
 * 010 111 0 0000000 100000000000000001 }- 0x5c020001
 * \ / \ / | \--+--/ \-------+--------/
 *  |   |  |    |            +------------ Start at 0x200000000
 *  |   |  |    |                                   0x200010000
 *  |   |  |    +------------------------- 4kB Region Size
 *  |   |  |                               ((0 + 1) * 4kB)
 *  |   |  +------------------------------ 4k Page Size
 *  |   +--------------------------------- GPCS5
 *  |                                      GPCS7
 *  +------------------------------------- GP Bus Memory
 */
#define CONFIG_SYS_SC520_CF1_PAR		0x54020000
#define CONFIG_SYS_SC520_CF2_PAR		0x5c020001

/*-----------------------------------------------------------------------
 * PAR for Extra 16550 UART A - 8 bytes @ 0x013f8, GPCS0
 * PAR for Extra 16550 UART B - 8 bytes @ 0x012f8, GPCS3
 * PAR for Extra 16550 UART C - 8 bytes @ 0x011f8, GPCS4
 * PAR for Extra 16550 UART D - 8 bytes @ 0x010f8, GPCS5
 *
 * 001 000 0 000000111 0001001111111000 }- 0x200713f8
 * 001 011 0 000000111 0001001011111000 }- 0x2c0712f8
 * 001 011 0 000000111 0001001011111000 }- 0x300711f8
 * 001 011 0 000000111 0001001011111000 }- 0x340710f8
 * \ / \ / | \---+---/ \------+-------/
 *  |   |  |     |            +----------- Start at 0x013f8
 *  |   |  |     |                                  0x012f8
 *  |   |  |     |                                  0x011f8
 *  |   |  |     |                                  0x010f8
 *  |   |  |     +------------------------ 33 Bytes (32 + 1)
 *  |   |  +------------------------------ Ignored
 *  |   +--------------------------------- GPCS6
 *  +------------------------------------- GP Bus I/O
 */
#define CONFIG_SYS_SC520_UARTA_PAR		0x200713f8
#define CONFIG_SYS_SC520_UARTB_PAR		0x2c0712f8
#define CONFIG_SYS_SC520_UARTC_PAR		0x300711f8
#define CONFIG_SYS_SC520_UARTD_PAR		0x340710f8

/*-----------------------------------------------------------------------
 * PAR for StrataFlash #1 - 16MB @ 0x10000000, ROMCS1
 * PAR for StrataFlash #2 - 16MB @ 0x11000000, ROMCS2
 *
 * 101 0 1 0 1 00011111111 01000000000000 }- 0xaa3fd000
 * 110 0 1 0 1 00011111111 01000100000000 }- 0xca3fd100
 * \ / | | | | \----+----/ \-----+------/
 *  |  | | | |      |            +---------- Start at 0x10000000
 *  |  | | | |      |                                 0x11000000
 *  |  | | | |      +----------------------- 16MB Region Size
 *  |  | | | |                               ((255 + 1) * 64kB)
 *  |  | | | +------------------------------ 64kB Page Size
 *  |  | | +-------------------------------- Writes Enabled
 *  |  | +---------------------------------- Caching Disabled
 *  |  +------------------------------------ Execution Enabled
 *  +--------------------------------------- ROMCS1
 *                                           ROMCS2
 */
#define CONFIG_SYS_SC520_SF1_PAR		0xaa3fd000
#define CONFIG_SYS_SC520_SF2_PAR		0xca3fd100

/*-----------------------------------------------------------------------
 * PAR for SRAM #1 - 1MB @ 0x19000000, GPCS0
 * PAR for SRAM #2 - 1MB @ 0x19100000, GPCS3
 *
 * 010 000 1 00000001111 01100100000000 }- 0x4203d900
 * 010 011 1 00000001111 01100100010000 }- 0x4e03d910
 * \ / \ / | \----+----/ \-----+------/
 *  |   |  |      |            +---------- Start at 0x19000000
 *  |   |  |      |                                 0x19100000
 *  |   |  |      +----------------------- 1MB Region Size
 *  |   |  |                               ((15 + 1) * 64kB)
 *  |   |  +------------------------------ 64kB Page Size
 *  |   +--------------------------------- GPCS0
 *  |                                      GPCS3
 *  +------------------------------------- GP Bus Memory
 */
#define CONFIG_SYS_SC520_SRAM1_PAR		0x4203d900
#define CONFIG_SYS_SC520_SRAM2_PAR		0x4e03d910

/*-----------------------------------------------------------------------
 * PAR for Dual-Port RAM - 4kB @ 0x18100000, GPCS4
 *
 * 010 100 0 00000000 11000000100000000 }- 0x50018100
 * \ / \ / | \---+--/ \-------+-------/
 *  |   |  |     |            +----------- Start at 0x18100000
 *  |   |  |     +------------------------ 4kB Region Size
 *  |   |  |                               ((0 + 1) * 4kB)
 *  |   |  +------------------------------ 4kB Page Size
 *  |   +--------------------------------- GPCS4
 *  +------------------------------------- GP Bus Memory
 */
#define CONFIG_SYS_SC520_DPRAM_PAR		0x50018100

/*-----------------------------------------------------------------------
 * PAR for SDRAM - 128MB @ 0x00000000
 * 111 0 0 0 1 11111111111 00000000000000 }- 0xe3ffc000
 * \ / | | | | \----+----/ \-----+------/
 *  |  | | | |      |            +---------- Start at 0x00000000
 *  |  | | | |      +----------------------- 128MB Region Size
 *  |  | | | |                               ((2047 + 1) * 64kB)
 *  |  | | | +------------------------------ 64kB Page Size
 *  |  | | +-------------------------------- Writes Enabled
 *  |  | +---------------------------------- Caching Enabled
 *  |  +------------------------------------ Execution Enabled
 *  +--------------------------------------- SDRAM
 */
#define CONFIG_SYS_SC520_SDRAM_PAR		0xe3ffc000

#ifndef __ASSEMBLER__
extern unsigned long ip;

#define PRINTIP				asm ("call 0\n" \
					    "0:\n" \
					    "pop %%eax\n" \
					    "movl %%eax, %0\n" \
					    :"=r"(ip) \
					    : /* No Input Registers */ \
					    :"%eax"); \
					    printf("IP: 0x%08lx (File: %s, Line: %d)\n", ip, __FILE__, __LINE__);

#endif
#endif	/* __CONFIG_H */
