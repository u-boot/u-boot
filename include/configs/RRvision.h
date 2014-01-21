/*
 * (C) Copyright 2000, 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC823		1	/* This is a MPC823 CPU		*/
#define CONFIG_RRVISION		1	/* ...on a RRvision board	*/

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#define CONFIG_8xx_GCLK_FREQ 64000000

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		115200	/* console baudrate = 115kbps	*/
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_PREBOOT	"setenv stdout serial"

#undef	CONFIG_BOOTARGS
#define CONFIG_ETHADDR                00:50:C2:00:E0:70
#define CONFIG_OVERWRITE_ETHADDR_ONCE 1
#define CONFIG_IPADDR                 10.0.0.5
#define CONFIG_SERVERIP               10.0.0.2
#define CONFIG_NETMASK                255.0.0.0
#define CONFIG_ROOTPATH               "/opt/eldk/ppc_8xx"
#define CONFIG_BOOTCOMMAND            "run flash_self"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"addip=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}"	\
		":${gatewayip}:${netmask}:${hostname}:${netdev}:off\0"	\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"load=tftp 100000 /tftpboot/u-boot.bin\0"			\
	"update=protect off 1:0-8;era 1:0-8;"				\
		"cp.b 100000 40000000 ${filesize};"			\
		"setenv filesize;saveenv\0"				\
	"kernel_addr=40040000\0"					\
	"ramdisk_addr=40100000\0"					\
	"kernel_img=/tftpboot/uImage\0"					\
	"kernel_load=tftp 200000 ${kernel_img}\0"			\
	"net_nfs=run kernel_load nfsargs addip addtty;bootm\0"		\
	"flash_nfs=run nfsargs addip addtty;bootm ${kernel_addr}\0"	\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"


#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#undef	CONFIG_STATUS_LED		/* disturbs display		*/

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/


#ifdef CONFIG_LCD
#define CONFIG_MPC8XX_LCD
#else
#define CONFIG_VIDEO		1	/* To enable the video initialization */

/* Video related */
#define CONFIG_VIDEO_LOGO			1	/* Show the logo */
#define CONFIG_VIDEO_ENCODER_AD7179		1	/* Enable this encoder */
#define CONFIG_VIDEO_ENCODER_AD7179_ADDR	0x2A	/* ALSB to ground */
#endif

/* enable I2C and select the hardware/software driver */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0xFE
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
#define I2C_DELAY	udelay(1)	/* 1/4 I2C clock duration */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_DATE

#undef CONFIG_CMD_PCMCIA
#undef CONFIG_CMD_IDE


/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0x40000000
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
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

/* timeout values are in ticks = ms */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(120*CONFIG_SYS_HZ)	/* Timeout for Flash Erase	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	(1 * CONFIG_SYS_HZ)	/* Timeout for Flash Write	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET+CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

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
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#ifndef	CONFIG_CAN_DRIVER
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)
#else	/* we must activate GPL5 in the SIUMCR for CAN */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01)
#endif	/* CONFIG_CAN_DRIVER */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF | PISCR_PTE)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 */

/* for 64 MHz, we use a 16 MHz clock * 4 */
#define CONFIG_SYS_PLPRCR ( (4-1)<<PLPRCR_MF_SHIFT | PLPRCR_TEXPS | PLPRCR_TMIST )

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	(/* SCCR_TBS  | */ SCCR_RTSEL | SCCR_RTDIV    | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

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

#define CONFIG_IDE_PREINIT	1	/* Use preinit IDE hook */
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
/*#define	CONFIG_SYS_DER	0x2002000F*/
#define CONFIG_SYS_DER	0

/*
 * Init Memory Controller:
 *
 * BR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing:
 */
/* 66 MHz CPU - 66 MHz bus: ACS = 00, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 1 */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_ACS_DIV1  | OR_TRLX | OR_CSNT_SAM | \
				 OR_SCY_3_CLK | OR_EHTR | OR_BI)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V )

/*
 * BR2/3 and OR2/3 (SDRAM)
 *
 */
#define SDRAM_BASE2_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define SDRAM_BASE3_PRELIM	0x20000000	/* SDRAM bank #1	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000A00

#define CONFIG_SYS_OR2_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_SDRAM )
#define CONFIG_SYS_BR2_PRELIM	((SDRAM_BASE2_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

#ifndef	CONFIG_CAN_DRIVER
#define	CONFIG_SYS_OR3_PRELIM	CONFIG_SYS_OR2_PRELIM
#define CONFIG_SYS_BR3_PRELIM	((SDRAM_BASE3_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )
#else	/* CAN uses CS3#, so we can have only one SDRAM bank anyway */
#define	CONFIG_SYS_CAN_BASE		0xC0000000	/* CAN mapped at 0xC0000000	*/
#define CONFIG_SYS_CAN_OR_AM		0xFFFF8000	/* 32 kB address mask		*/
#define CONFIG_SYS_OR3_CAN		(CONFIG_SYS_CAN_OR_AM | OR_G5LA | OR_BI)
#define CONFIG_SYS_BR3_CAN		((CONFIG_SYS_CAN_BASE & BR_BA_MSK) | \
					BR_PS_8 | BR_MS_UPMB | BR_V )
#endif	/* CONFIG_CAN_DRIVER */

/*
 * Memory Periodic Timer Prescaler
 *
 * The Divider for PTA (refresh timer) configuration is based on an
 * example SDRAM configuration (64 MBit, one bank). The adjustment to
 * the number of chip selects (NCS) and the actually needed refresh
 * rate is done by setting MPTPR.
 *
 * PTA is calculated from
 *	PTA = (gclk * Trefresh) / ((2 ^ (2 * DFBRG)) * PTP * NCS)
 *
 *	gclk	  CPU clock (not bus clock!)
 *	Trefresh  Refresh cycle * 4 (four word bursts used)
 *
 * 4096  Rows from SDRAM example configuration
 * 1000  factor s -> ms
 *   32  PTP (pre-divider from MPTPR) from SDRAM example configuration
 *    4  Number of refresh cycles per period
 *   64  Refresh cycle in ms per number of rows
 * --------------------------------------------
 * Divider = 4096 * 32 * 1000 / (4 * 64) = 512000
 *
 * 50 MHz => 50.000.000 / Divider =  98
 * 66 Mhz => 66.000.000 / Divider = 129
 * 80 Mhz => 80.000.000 / Divider = 156
 */
#define CONFIG_SYS_MAMR_PTA		129

/*
 * For 16 MBit, refresh rates could be 31.3 us
 * (= 64 ms / 2K = 125 / quad bursts).
 * For a simpler initialization, 15.6 us is used instead.
 *
 * #define CONFIG_SYS_MPTPR_2BK_2K	MPTPR_PTP_DIV32		for 2 banks
 * #define CONFIG_SYS_MPTPR_1BK_2K	MPTPR_PTP_DIV64		for 1 bank
 */
#define CONFIG_SYS_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CONFIG_SYS_MPTPR_1BK_4K	MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CONFIG_SYS_MPTPR_2BK_8K	MPTPR_PTP_DIV8		/* setting for 2 banks	*/
#define CONFIG_SYS_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CONFIG_SYS_MAMR_8COL	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)
/* 9 column SDRAM */
#define CONFIG_SYS_MAMR_9COL	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)


#endif	/* __CONFIG_H */
