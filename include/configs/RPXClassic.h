/*
 * (C) Copyright 2000, 2001, 2002
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

/*
 * board/config.h - configuration options, board specific
 */

/* Yoo. Jonghoon, IPone, yooth@ipone.co.kr
 * U-Boot port on RPXlite board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define	RPXClassic_50MHz

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC860           1
#define CONFIG_RPXCLASSIC		1

#define	CONFIG_SYS_TEXT_BASE	0xff000000

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600	/* console baudrate = 9600bps	*/

/* Define CONFIG_FEC_ENET to use Fast ethernet instead of ethernet on SCC1   */
#define CONFIG_FEC_ENET
#ifdef CONFIG_FEC_ENET
#define CONFIG_SYS_DISCOVER_PHY        1
#define CONFIG_MII              1
#endif /* CONFIG_FEC_ENET */
#define CONFIG_MISC_INIT_R

/* Video console (graphic: Epson SED13806 on ECCX board, no keyboard         */
#if 1
#define CONFIG_VIDEO_SED13806
#define CONFIG_NEC_NL6448BC20
#define CONFIG_VIDEO_SED13806_16BPP

#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_CONSOLE_EXTRA_INFO
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_SW_CURSOR
#endif

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_ZERO_BOOTDELAY_CHECK 1

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"tftpboot; "								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_RESET_ADDRESS	0x80000000
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0040000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00C0000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFA200000

/*-----------------------------------------------------------------------------
 * I2C Configuration
 *-----------------------------------------------------------------------------
 */
#define CONFIG_I2C              1
#define CONFIG_SYS_I2C_SPEED           50000
#define CONFIG_SYS_I2C_SLAVE           0x34


/* enable I2C and select the hardware/software driver */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef  CONFIG_SOFT_I2C			/* I2C bit-banged		*/
/*
 * Software (bit-bang) I2C driver configuration
 */
#define I2C_PORT	1		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00000010)
#define I2C_TRISTATE	(iop->pdir &= ~0x00000010)
#define I2C_READ	((iop->pdat & 0x00000010) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00000010; \
			else    iop->pdat &= ~0x00000010
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00000020; \
			else    iop->pdat &= ~0x00000020
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */


# define CONFIG_SYS_I2C_SPEED		50000
# define CONFIG_SYS_I2C_SLAVE		0x34
# define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM X24C16		*/
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x3000	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE	0xFF000000

#if defined(DEBUG) || defined (CONFIG_VIDEO_SED13806) || defined(CONFIG_CMD_IDE)
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
#endif
#define CONFIG_SYS_MONITOR_BASE	0xFF000000
/*%%% #define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE */
#define	CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	71	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#if 0
#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_OFFSET		0x20000	/*   Offset   of Environment Sector  */
#define CONFIG_ENV_SECT_SIZE       0x8000
#define	CONFIG_ENV_SIZE		0x8000	/* Total Size of Environment Sector  */
#else
#define CONFIG_ENV_IS_IN_NVRAM     1
#define CONFIG_ENV_ADDR            0xfa000100
#define CONFIG_ENV_SIZE            0x1000
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWP)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_MLRC10)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
/*%%%#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE) */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC |  RTCSC_ALR | RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR (PISCR_PS | PISCR_PITF | PISCR_PTE)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 *
 * If this is a 80 MHz CPU, set PLL multiplication factor to 5 (5*16=80)!
 */
/* up to 50 MHz we use a 1:1 clock */
#define CONFIG_SYS_PLPRCR	( (4 << PLPRCR_MF_SHIFT) | PLPRCR_TEXPS | PLPRCR_SPLSS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
/* up to 50 MHz we use a 1:1 clock */
#define CONFIG_SYS_SCCR	(SCCR_COM00 | SCCR_TBS)

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_PCMCIA_MEM_ADDR	(0xE0000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR	(0xE4000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_IO_ADDR	(0xEC000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define	CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(2 * CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0100

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/* #define	CONFIG_SYS_DER	0x2002000F */
#define CONFIG_SYS_DER	0

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */

#define FLASH_BASE_PRELIM	0xFE000000	/* FLASH base */
#define CONFIG_SYS_PRELIM_OR_AM	0xFE000000	/* OR addr mask */

/* FLASH timing: ACS = 0, TRLX = 0, CSNT = 0, SCY = 4, ETHR = 0, BIH = 1 */
#define CONFIG_SYS_OR_TIMING_FLASH (OR_SCY_4_CLK | OR_BI)

#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE_PRELIM & BR_BA_MSK) | BR_V)

/*
 * BR1 and OR1 (SDRAM)
 *
 */
#define SDRAM_BASE_PRELIM	0x00000000	/* SDRAM base	*/
#define	SDRAM_MAX_SIZE		0x01000000	/* max 16 MB */

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000E00

#define CONFIG_SYS_OR1_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_SDRAM )
#define CONFIG_SYS_BR1_PRELIM	((SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/* RPXLITE mem setting */
#define	CONFIG_SYS_BR3_PRELIM	0xFA400001		/* BCSR */
#define CONFIG_SYS_OR3_PRELIM	0xff7f8970
#define	CONFIG_SYS_BR4_PRELIM	0xFA000401		/* NVRAM&SRAM */
#define CONFIG_SYS_OR4_PRELIM	0xFFF80970

/* ECCX CS settings                                                          */
#define SED13806_OR             0xFFC00108     /* - 4 Mo
						   - Burst inhibit
						   - external TA             */
#define SED13806_REG_ADDR       0xa0000000
#define SED13806_ACCES          0x801           /* 16 bit access             */


/* Global definitions for the ECCX board                                     */
#define ECCX_CSR_ADDR           (0xfac00000)
#define ECCX_CSR8_OFFSET        (0x8)
#define ECCX_CSR11_OFFSET       (0xB)
#define ECCX_CSR12_OFFSET       (0xC)

#define ECCX_CSR8  (volatile unsigned char *)(ECCX_CSR_ADDR + ECCX_CSR8_OFFSET)
#define ECCX_CSR11 (volatile unsigned char *)(ECCX_CSR_ADDR + ECCX_CSR11_OFFSET)
#define ECCX_CSR12 (volatile unsigned char *)(ECCX_CSR_ADDR + ECCX_CSR12_OFFSET)


#define REG_GPIO_CTRL 0x008

/* Definitions for CSR8                                                      */
#define ECCX_ENEPSON            0x80    /* Bit 0:
					   0= disable and reset SED1386
					   1= enable SED1386                 */
/* Bit 1:   0= SED1386 in Big Endian mode                                    */
/*          1= SED1386 in little endian mode                                 */
#define ECCX_LE                 0x40
#define ECCX_BE                 0x00

/* Bit 2,3: Selection                                                        */
/*      00 = Disabled                                                        */
/*      01 = CS2 is used for the SED1386                                     */
/*      10 = CS5 is used for the SED1386                                     */
/*      11 = reserved                                                        */
#define ECCX_CS2                0x10
#define ECCX_CS5                0x20

/* Definitions for CSR12                                                     */
#define ECCX_ID                 0x02
#define ECCX_860                0x01

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA	58

/*
 * Refresh clock Prescalar
 */
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV8

/*
 * MAMR settings for SDRAM
 */

/* 10 column SDRAM */
#define CONFIG_SYS_MAMR_10COL	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_2 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A12 |	\
			 MAMR_GPL_A4DIS | MAMR_RLFA_4X | MAMR_WLFA_3X | MAMR_TLFA_16X)

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* Configuration variable added by yooth. */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/*
 * BCSRx
 *
 * Board Status and Control Registers
 *
 */

#define BCSR0 0xFA400000
#define BCSR1 0xFA400001
#define BCSR2 0xFA400002
#define BCSR3 0xFA400003

#define BCSR0_ENMONXCVR	0x01	/* Monitor XVCR Control */
#define BCSR0_ENNVRAM	0x02	/* CS4# Control */
#define BCSR0_LED5		0x04	/* LED5 control 0='on' 1='off' */
#define BCSR0_LED4		0x08	/* LED4 control 0='on' 1='off' */
#define BCSR0_FULLDPLX	0x10	/* Ethernet XCVR Control */
#define BCSR0_COLTEST	0x20
#define BCSR0_ETHLPBK	0x40
#define BCSR0_ETHEN	0x80

#define BCSR1_PCVCTL7	0x01	/* PC Slot B Control */
#define BCSR1_PCVCTL6	0x02
#define BCSR1_PCVCTL5	0x04
#define BCSR1_PCVCTL4	0x08
#define BCSR1_IPB5SEL	0x10

#define BCSR2_MIIRST    0x80
#define BCSR2_MIIPWRDWN 0x40
#define BCSR2_MIICTL    0x08

#define BCSR3_BWRTC		0x01	/* Real Time Clock Battery */
#define BCSR3_BWNVR		0x02	/* NVRAM Battery */
#define BCSR3_RDY_BSY	0x04	/* Flash Operation */
#define BCSR3_RPXL		0x08	/* Reserved (reads back '1') */
#define BCSR3_D27		0x10	/* Dip Switch settings */
#define BCSR3_D26		0x20
#define BCSR3_D25		0x40
#define BCSR3_D24		0x80


/*
 * Environment setting
 */

/* #define CONFIG_ETHADDR	00:10:EC:00:2C:A2 */
/* #define CONFIG_IPADDR	10.10.106.1 */
/* #define CONFIG_SERVERIP	10.10.104.11 */

#endif	/* __CONFIG_H */
