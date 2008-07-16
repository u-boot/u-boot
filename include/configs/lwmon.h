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
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* External logbuffer support */
#define CONFIG_LOGBUFFER

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC823		1	/* This is a MPC823E CPU	*/
#define CONFIG_LWMON		1	/* ...on a LWMON board		*/

/* Default Ethernet MAC address */
#define CONFIG_ETHADDR          00:11:B0:00:00:00

/* The default Ethernet MAC address can be overwritten just once */
#ifdef CONFIG_ETHADDR
#define CONFIG_OVERWRITE_ETHADDR_ONCE   1
#endif

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/
#define CONFIG_BOARD_POSTCLK_INIT 1	/* Call board_postclk_init	*/

#define CONFIG_LCD		1	/* use LCD controller ...	*/
#define CONFIG_HLD1045		1	/* ... with a HLD1045 display	*/

#define CONFIG_LCD_LOGO		1	/* print our logo on the LCD	*/
#define CONFIG_LCD_INFO		1	/* ... and some board info	*/
#define	CONFIG_SPLASH_SCREEN		/* ... with splashscreen support*/

#define CONFIG_SERIAL_MULTI	1
#define CONFIG_8xx_CONS_SMC2	1	/* Console is on SMC2		*/
#define CONFIG_8xx_CONS_SCC2	1	/* Console is on SCC2		*/

#define CONFIG_BAUDRATE		115200	/* with watchdog >= 38400 needed */

#define CONFIG_BOOTDELAY	1	/* autoboot after 1 second	*/

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

/* pre-boot commands */
#define	CONFIG_PREBOOT		"setenv bootdelay 15"

#undef	CONFIG_BOOTARGS

/* POST support */
#define CONFIG_POST		(CFG_POST_CACHE	   | \
				 CFG_POST_WATCHDOG | \
				 CFG_POST_RTC	   | \
				 CFG_POST_MEMORY   | \
				 CFG_POST_CPU	   | \
				 CFG_POST_UART	   | \
				 CFG_POST_ETHER    | \
				 CFG_POST_I2C	   | \
				 CFG_POST_SPI	   | \
				 CFG_POST_USB	   | \
				 CFG_POST_SPR	   | \
				 CFG_POST_SYSMON)

/*
 * Keyboard commands:
 * # = 0x28 = ENTER :		enable bootmessages on LCD
 * 2 = 0x3A+0x3C = F1 + F3 :	enable update mode
 * 3 = 0x3C+0x3F = F3 + F6 :	enable test mode
 */

#define CONFIG_BOOTCOMMAND "autoscr 40040000;saveenv"

/*	"gatewayip=10.8.211.250\0"			                \ */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"kernel_addr=40080000\0"					\
	"ramdisk_addr=40280000\0"					\
	"netmask=255.255.192.0\0"				        \
	"serverip=10.8.2.101\0"				                \
	"ipaddr=10.8.57.0\0"				                \
	"magic_keys=#23\0"						\
	"key_magic#=28\0"						\
	"key_cmd#=setenv addfb setenv 'bootargs $bootargs console=tty0'\0" \
	"key_magic2=3A+3C\0"						\
	"key_cmd2=echo *** Entering Update Mode ***;"			\
		"if fatload ide 0:3 10000 update.scr;"			\
			"then autoscr 10000;"				\
			"else echo *** UPDATE FAILED ***;"		\
		"fi\0"							\
	"key_magic3=3C+3F\0"						\
	"key_cmd3=echo *** Entering Test Mode ***;"			\
		"setenv add_misc 'setenv bootargs $bootargs testmode'\0" \
	"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath\0" \
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addfb=setenv bootargs $bootargs console=ttyS1,$baudrate\0"	\
	"addip=setenv bootargs $bootargs "				\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname::off " \
		"panic=1\0"						\
	"add_wdt=setenv bootargs $bootargs $wdt_args\0"			\
	"add_misc=setenv bootargs $bootargs runmode\0"			\
	"flash_nfs=run nfsargs addip add_wdt addfb add_misc;"		\
		"bootm $kernel_addr\0"					\
	"flash_self=run ramargs addip add_wdt addfb add_misc;"		\
		"bootm $kernel_addr $ramdisk_addr\0"			\
	"net_nfs=tftp 100000 /tftpboot/uImage.lwmon;"			\
		"run nfsargs addip add_wdt addfb;bootm\0"		\
	"rootpath=/opt/eldk/ppc_8xx\0"					\
	"load=tftp 100000 /tftpboot/u-boot.bin\0"			\
	"update=protect off 1:0;era 1:0;cp.b 100000 40000000 $filesize\0" \
	"wdt_args=wdt_8xx=off\0"					\
	"verify=no"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#define	CONFIG_WATCHDOG		1	/* watchdog enabled		*/
#define	CFG_WATCHDOG_FREQ       (CFG_HZ / 20)

#undef	CONFIG_STATUS_LED		/* Status LED disabled		*/

/* enable I2C and select the hardware/software driver */
#undef	CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define	CONFIG_SOFT_I2C         1	/* I2C bit-banged		*/

#define CFG_I2C_SPEED		93000	/* 93 kHz is supposed to work	*/
#define CFG_I2C_SLAVE		0xFE

#ifdef CONFIG_SOFT_I2C
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL		0x00000020	/* PB 26 */
#define PB_SDA		0x00000010	/* PB 27 */

#define I2C_INIT	(immr->im_cpm.cp_pbdir |=  PB_SCL)
#define I2C_ACTIVE	(immr->im_cpm.cp_pbdir |=  PB_SDA)
#define I2C_TRISTATE	(immr->im_cpm.cp_pbdir &= ~PB_SDA)
#define I2C_READ	((immr->im_cpm.cp_pbdat & PB_SDA) != 0)
#define I2C_SDA(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SDA; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SDA
#define I2C_SCL(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SCL; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SCL
#define I2C_DELAY	udelay(2)	/* 1/4 I2C clock duration */
#endif	/* CONFIG_SOFT_I2C */


#define CONFIG_RTC_PCF8563		/* use Philips PCF8563 RTC	*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BMP
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP

#ifdef CONFIG_POST
#define CONFIG_CMD_DIAG
#endif


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define	CFG_HUSH_PARSER		1	/* use "hush" command parser	*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00F00000	/* 1 ... 15MB in DRAM	*/

#define CFG_LOAD_ADDR		0x00100000	/* default load address */

#define CFG_PIO_MODE		0	/* IDE interface in PIO Mode 0	*/

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * When the watchdog is enabled, output must be fast enough in Linux.
 */
#ifdef CONFIG_WATCHDOG
#define CFG_BAUDRATE_TABLE	{		38400, 57600, 115200 }
#else
#define CFG_BAUDRATE_TABLE	{  9600, 19200, 38400, 57600, 115200 }
#endif

/*----------------------------------------------------------------------*/
#define CONFIG_MODEM_SUPPORT	1	/* enable modem initialization stuff */
#undef CONFIG_MODEM_SUPPORT_DEBUG

#define	CONFIG_MODEM_KEY_MAGIC	"3C+3D"	/* press F3 + F4 keys to enable modem */
#define	CONFIG_POST_KEY_MAGIC	"3C+3E"	/* press F3 + F5 keys to force POST */
#if 0
#define	CONFIG_AUTOBOOT_KEYED		/* Enable "password" protection	*/
#define CONFIG_AUTOBOOT_PROMPT	\
	"\nEnter password - autoboot in %d sec...\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	"  "	/* "password"	*/
#endif
/*----------------------------------------------------------------------*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	68  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x40000000
#if defined(DEBUG) || defined(CONFIG_CMD_IDE)
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define CFG_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
#endif
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

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
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	180000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	600	/* Timeout for Flash Write (in ms)	*/
#define CFG_FLASH_USE_BUFFER_WRITE
#define CFG_FLASH_BUFFER_WRITE_TOUT	2048	/* Timeout for Flash Buffer Write (in ms)	*/
/* Buffer size.
   We have two flash devices connected in parallel.
   Each device incorporates a Write Buffer of 32 bytes.
 */
#define CFG_FLASH_BUFFER_SIZE	(2*32)

/* Put environment in flash which is much faster to boot than using the EEPROM	*/
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR	    0x40040000	/* Address    of Environment Sector	*/
#define CFG_ENV_SIZE		0x2000	/* Total Size of Environment		*/
#define CFG_ENV_SECT_SIZE	0x40000 /* we have BIG sectors only :-(		*/

/*-----------------------------------------------------------------------
 * I2C/EEPROM Configuration
 */

#define CFG_I2C_AUDIO_ADDR	0x28	/* Audio volume control			*/
#define CFG_I2C_SYSMON_ADDR	0x2E	/* LM87 System Monitor			*/
#define CFG_I2C_RTC_ADDR	0x51	/* PCF8563 RTC				*/
#define CFG_I2C_POWER_A_ADDR	0x52	/* PCMCIA/USB power switch, channel A	*/
#define CFG_I2C_POWER_B_ADDR	0x53	/* PCMCIA/USB power switch, channel B	*/
#define CFG_I2C_KEYBD_ADDR	0x56	/* PIC LWE keyboard			*/
#define CFG_I2C_PICIO_ADDR	0x57	/* PIC IO Expander			*/

#undef	CONFIG_USE_FRAM			/* Use FRAM instead of EEPROM	*/

#ifdef CONFIG_USE_FRAM	/* use FRAM */
#define CFG_I2C_EEPROM_ADDR	0x55	/* FRAM FM24CL64		*/
#define CFG_I2C_EEPROM_ADDR_LEN	2
#else			/* use EEPROM */
#define CFG_I2C_EEPROM_ADDR	0x58	/* EEPROM AT24C164		*/
#define CFG_I2C_EEPROM_ADDR_LEN	1
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* takes up to 10 msec	*/
#endif	/* CONFIG_USE_FRAM */
#define CFG_EEPROM_PAGE_WRITE_BITS	4

/* List of I2C addresses to be verified by POST */
#ifdef CONFIG_USE_FRAM
#define I2C_ADDR_LIST	{  /*	CFG_I2C_AUDIO_ADDR, */	\
				CFG_I2C_SYSMON_ADDR,	\
				CFG_I2C_RTC_ADDR,	\
				CFG_I2C_POWER_A_ADDR,	\
				CFG_I2C_POWER_B_ADDR,	\
				CFG_I2C_KEYBD_ADDR,	\
				CFG_I2C_PICIO_ADDR,	\
				CFG_I2C_EEPROM_ADDR,	\
			}
#else	/* Use EEPROM - which show up on 8 consequtive addresses */
#define I2C_ADDR_LIST	{  /*	CFG_I2C_AUDIO_ADDR, */	\
				CFG_I2C_SYSMON_ADDR,	\
				CFG_I2C_RTC_ADDR,	\
				CFG_I2C_POWER_A_ADDR,	\
				CFG_I2C_POWER_B_ADDR,	\
				CFG_I2C_KEYBD_ADDR,	\
				CFG_I2C_PICIO_ADDR,	\
				CFG_I2C_EEPROM_ADDR+0,	\
				CFG_I2C_EEPROM_ADDR+1,	\
				CFG_I2C_EEPROM_ADDR+2,	\
				CFG_I2C_EEPROM_ADDR+3,	\
				CFG_I2C_EEPROM_ADDR+4,	\
				CFG_I2C_EEPROM_ADDR+5,	\
				CFG_I2C_EEPROM_ADDR+6,	\
				CFG_I2C_EEPROM_ADDR+7,	\
			}
#endif	/* CONFIG_USE_FRAM */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if 0 && defined(CONFIG_WATCHDOG)	/* LWMON uses external MAX706TESA WD */
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
/* EARB, DBGC and DBPC are initialised by the HCW */
/* => 0x000000C0 */
#define CFG_SIUMCR	(SIUMCR_GB5E)
/*#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01) */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit, set PLL multiplication factor !
 */
/* 0x00405000 */
#define CFG_PLPRCR_MF	4	/* (4+1) * 13.2 = 66 MHz Clock */
#define CFG_PLPRCR							\
		(	(CFG_PLPRCR_MF << PLPRCR_MF_SHIFT) |		\
			PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST |	\
			/*PLPRCR_CSRC|*/ PLPRCR_LPM_NORMAL |		\
			PLPRCR_CSR    /*| PLPRCR_LOLRE|PLPRCR_FIOPD*/	\
		)

#define CONFIG_8xx_GCLK_FREQ	((CFG_PLPRCR_MF+1)*13200000)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
/* 0x01800000 */
#define CFG_SCCR	(SCCR_COM00	| /*SCCR_TBS|*/		\
			 SCCR_RTDIV	|   SCCR_RTSEL	  |	\
			 /*SCCR_CRQEN|*/  /*SCCR_PRQEN|*/	\
			 SCCR_EBDF00 |	 SCCR_DFSYNC00 |	\
			 SCCR_DFBRG00	|   SCCR_DFNL000  |	\
			 SCCR_DFNH000	|   SCCR_DFLCD100 |	\
			 SCCR_DFALCD01)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
/* 0x00C3 => 0x0003 */
#define CFG_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)


/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration Register		19-4
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR 0x0000

/*-----------------------------------------------------------------------
 * RMDS - RISC Microcode Development Support Control Register
 *-----------------------------------------------------------------------
 */
#define CFG_RMDS 0

/*-----------------------------------------------------------------------
 *
 * Interrupt Levels
 *-----------------------------------------------------------------------
 */
#define CFG_CPM_INTERRUPT	13	/* SIU_LEVEL6	*/

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CFG_PCMCIA_MEM_ADDR	(0x50000000)
#define CFG_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CFG_PCMCIA_DMA_ADDR	(0x54000000)
#define CFG_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CFG_PCMCIA_ATTRB_ADDR	(0x58000000)
#define CFG_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CFG_PCMCIA_IO_ADDR	(0x5C000000)
#define CFG_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#undef	CONFIG_IDE_LED			/* LED	 for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	CFG_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	(CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(2 * CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	0x0100

#define CONFIG_SUPPORT_VFAT		/* enable VFAT support */

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER 0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH) - second Flash bank optional
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0x41000000	/* FLASH bank #1	*/

/* used to re-map FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0xFF000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xFF000000	/* OR addr mask */

/* FLASH timing: ACS = 00, TRLX = 0, CSNT = 1, SCY = 8, EHTR = 0	*/
#define CFG_OR_TIMING_FLASH	(OR_SCY_8_CLK)

#define CFG_OR0_REMAP	( CFG_REMAP_OR_AM | OR_CSNT_SAM | OR_ACS_DIV1 | OR_BI | \
				CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | OR_ACS_DIV1 | OR_BI | \
				CFG_OR_TIMING_FLASH)
/* 16 bit, bank valid */
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_32 | BR_V )

#define CFG_OR1_REMAP	CFG_OR0_REMAP
#define CFG_OR1_PRELIM	CFG_OR0_PRELIM
#define CFG_BR1_PRELIM	((FLASH_BASE1_PRELIM & BR_BA_MSK) | BR_PS_32 | BR_V )

/*
 * BR3/OR3: SDRAM
 *
 * Multiplexed addresses, GPL5 output to GPL5_A (don't care)
 */
#define SDRAM_BASE3_PRELIM	0x00000000	/* SDRAM bank */
#define SDRAM_PRELIM_OR_AM	0xF0000000	/* map 256 MB (>SDRAM_MAX_SIZE!) */
#define SDRAM_TIMING		OR_SCY_0_CLK	/* SDRAM-Timing */

#define SDRAM_MAX_SIZE		0x08000000	/* max 128 MB SDRAM */

#define CFG_OR3_PRELIM	(SDRAM_PRELIM_OR_AM | OR_CSNT_SAM | OR_G5LS | SDRAM_TIMING )
#define CFG_BR3_PRELIM	((SDRAM_BASE3_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/*
 * BR5/OR5: Touch Panel
 *
 * AM=0xFFC00 ATM=0 CSNT/SAM=0 ACS/G5LA/G5LS=3 BIH=1 SCY=0 SETA=0 TRLX=0 EHTR=0
 */
#define TOUCHPNL_BASE		0x20000000
#define TOUCHPNL_OR_AM		0xFFFF8000
#define TOUCHPNL_TIMING		OR_SCY_0_CLK

#define CFG_OR5_PRELIM	(TOUCHPNL_OR_AM | OR_CSNT_SAM | OR_ACS_DIV1 | OR_BI | \
			 TOUCHPNL_TIMING )
#define CFG_BR5_PRELIM	((TOUCHPNL_BASE & BR_BA_MSK) | BR_PS_32 | BR_V )

#define	CFG_MEMORY_75
#undef	CFG_MEMORY_7E
#undef	CFG_MEMORY_8E

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MPTPR	0x200

/*
 * MAMR settings for SDRAM
 */

#define CFG_MAMR_8COL	0x80802114
#define CFG_MAMR_9COL	0x80904114

/*
 * MAR setting for SDRAM
 */
#define CFG_MAR		0x00000088

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */
