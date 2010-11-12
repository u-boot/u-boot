/*
 * (C) Copyright 2006
 * Markus Klotzbuecher, DENX Software Engineering, mk@denx.de
 *
 * Configuation settings for the SPC1920 board.
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

#ifndef __H
#define __CONFIG_H

#define CONFIG_SPC1920			1	/* SPC1920 board */
#define CONFIG_MPC885			1	/* MPC885 CPU */

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

#define	CONFIG_8xx_CONS_SMC1		/* Console is on SMC1 */
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE

#define CONFIG_MII
#define CONFIG_MII_INIT		1
#undef CONFIG_ETHER_ON_FEC1
#define CONFIG_ETHER_ON_FEC2
#define FEC_ENET
#define CONFIG_FEC2_PHY		1

#define CONFIG_BAUDRATE		19200

/* use PLD CLK4 instead of brg */
#define CONFIG_SYS_SPC1920_SMC1_CLK4

#define CONFIG_8xx_OSCLK		10000000 /* 10 MHz oscillator on EXTCLK  */
#define CONFIG_8xx_CPUCLK_DEFAULT	50000000
#define CONFIG_SYS_8xx_CPUCLK_MIN		40000000
#define CONFIG_SYS_8xx_CPUCLK_MAX		133000000

#define CONFIG_SYS_RESET_ADDRESS		0xC0000000

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_LAST_STAGE_INIT

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_ENV_OVERWRITE

#define CONFIG_NFSBOOTCOMMAND							\
    "dhcp;"									\
    "setenv bootargs root=/dev/nfs rw nfsroot=$rootpath "			\
    "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:eth0:off;"		\
    "bootm"

#define CONFIG_BOOTCOMMAND							\
    "setenv bootargs root=/dev/mtdblock2 rw mtdparts=phys:1280K(ROM)ro,-(root) "\
    "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:eth0:off;"		\
    "bootm fe080000"

#undef CONFIG_BOOTARGS

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#define CONFIG_BZIP2	 /* include support for bzip2 compressed images */


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
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"=>"		/* Monitor Command Prompt	*/
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif

#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16) /* Print Buffer Size	*/
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define	CONFIG_SYS_HZ		        1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 2400, 4800, 9600, 19200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xF0000000

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
#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 KB for monitor	*/

#ifdef CONFIG_BZIP2
#define	CONFIG_SYS_MALLOC_LEN		(2500 << 10)	/* Reserve ~2.5 MB for malloc()	*/
#else
#define	CONFIG_SYS_MALLOC_LEN		(384 << 10)	/* Reserve 384 kB for malloc()	*/
#endif /* CONFIG_BZIP2 */

#define	CONFIG_SYS_ALLOC_DPRAM		1	/* use allocation routines	*/

/*
 * Flash
 */
/*-----------------------------------------------------------------------
 * Flash organisation
 */
#define CONFIG_SYS_FLASH_BASE          0xFE000000
#define CONFIG_SYS_FLASH_CFI                           /* The flash is CFI compatible  */
#define CONFIG_FLASH_CFI_DRIVER                    /* Use common CFI driver        */
#define CONFIG_SYS_MAX_FLASH_BANKS     1               /* Max number of flash banks    */
#define CONFIG_SYS_MAX_FLASH_SECT      128             /* Max num of sects on one chip */

/* Environment is in flash */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE       0x40000         /* We use one complete sector   */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

#define CONFIG_ENV_OVERWRITE

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/

#ifdef CONFIG_CMD_DATE
# define CONFIG_RTC_DS3231
# define CONFIG_SYS_I2C_RTC_ADDR      0x68
#endif

/*-----------------------------------------------------------------------
 * I2C configuration
 */
#if defined(CONFIG_CMD_I2C)
/* enable I2C and select the hardware/software driver */
#undef CONFIG_HARD_I2C                 /* I2C with hardware support    */
#define CONFIG_SOFT_I2C                1       /* I2C bit-banged               */

#define CONFIG_SYS_I2C_SPEED          93000   /* 93 kHz is supposed to work   */
#define CONFIG_SYS_I2C_SLAVE          0xFE

#ifdef CONFIG_SOFT_I2C
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL         0x00000020      /* PB 26 */
#define PB_SDA         0x00000010      /* PB 27 */

#define I2C_INIT       (immr->im_cpm.cp_pbdir |=  PB_SCL)
#define I2C_ACTIVE     (immr->im_cpm.cp_pbdir |=  PB_SDA)
#define I2C_TRISTATE   (immr->im_cpm.cp_pbdir &= ~PB_SDA)
#define I2C_READ       ((immr->im_cpm.cp_pbdat & PB_SDA) != 0)
#define I2C_SDA(bit)   if(bit) immr->im_cpm.cp_pbdat |=  PB_SDA; \
		       else    immr->im_cpm.cp_pbdat &= ~PB_SDA
#define I2C_SCL(bit)   if(bit) immr->im_cpm.cp_pbdat |=  PB_SCL; \
		       else    immr->im_cpm.cp_pbdat &= ~PB_SCL
#define I2C_DELAY      udelay(2)       /* 1/4 I2C clock duration */
#endif /* CONFIG_SOFT_I2C */
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
#define CONFIG_SYS_SIUMCR      (SIUMCR_FRC)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
/* #define CONFIG_SYS_SCCR	SCCR_TBS */
#define CONFIG_SYS_SCCR	(SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

/*-----------------------------------------------------------------------
 * DER - Debug Enable Register
 *-----------------------------------------------------------------------
 * Set to zero to prevent the processor from entering debug mode
 */
#define CONFIG_SYS_DER		 0


/* Because of the way the 860 starts up and assigns CS0 the entire
 * address space, we have to set the memory controller differently.
 * Normally, you write the option register first, and then enable the
 * chip select by writing the base register.  For CS0, you must write
 * the base register first, followed by the option register.
 */


/*
 * Init Memory Controller:
 */

/* BR0 and OR0 (FLASH) */
#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE	/* FLASH bank #0 */


/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing:
 */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_ACS_DIV1  | OR_TRLX | OR_CSNT_SAM | \
				 OR_SCY_6_CLK | OR_EHTR | OR_BI)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V )


/*
 * SDRAM CS1 UPMB
 */
#define	CONFIG_SYS_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE_PRELIM CONFIG_SYS_SDRAM_BASE
#define SDRAM_MAX_SIZE	0x4000000 /* max 64 MB */

#define CONFIG_SYS_PRELIM_OR1_AM	0xF0000000
/* #define CONFIG_SYS_OR1_TIMING  OR_CSNT_SAM/\*  | OR_G5LS /\\* *\\/ *\/ */
#define SDRAM_TIMING	OR_SCY_0_CLK	/* SDRAM-Timing */

#define CONFIG_SYS_OR1_PRELIM	(CONFIG_SYS_PRELIM_OR1_AM | OR_CSNT_SAM | OR_G5LS | SDRAM_TIMING)
#define CONFIG_SYS_BR1_PRELIM  ((CONFIG_SYS_SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMB | BR_V)

/* #define CONFIG_SYS_OR1_FINAL   ((CONFIG_SYS_OR1_AM & OR_AM_MSK) | CONFIG_SYS_OR1_TIMING) */
/* #define CONFIG_SYS_BR1_FINAL   ((CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMB | BR_V) */

#define CONFIG_SYS_PTB_PER_CLK	((4096 * 16 * 1000) / (4 * 64))
#define CONFIG_SYS_PTA_PER_CLK 195
#define CONFIG_SYS_MBMR_PTB	195
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV16
#define CONFIG_SYS_MAR		0x88

#define CONFIG_SYS_MBMR_8COL  ((CONFIG_SYS_MBMR_PTB << MBMR_PTB_SHIFT) | \
			MBMR_AMB_TYPE_0 | \
			MBMR_G0CLB_A10 | \
			MBMR_DSB_1_CYCL | \
			MBMR_RLFB_1X | \
			MBMR_WLFB_1X | \
			MBMR_TLFB_4X) /* 0x04804114 */ /* 0x10802114 */

#define CONFIG_SYS_MBMR_9COL  ((CONFIG_SYS_MBMR_PTB << MBMR_PTB_SHIFT) | \
			MBMR_AMB_TYPE_1 | \
			MBMR_G0CLB_A10 | \
			MBMR_DSB_1_CYCL | \
			MBMR_RLFB_1X | \
			MBMR_WLFB_1X | \
			MBMR_TLFB_4X) /* 0x04804114 */ /* 0x10802114 */


/*
 * DSP Host Port Interface CS3
 */
#define CONFIG_SYS_SPC1920_HPI_BASE   0x90000000
#define CONFIG_SYS_PRELIM_OR3_AM      0xF8000000

#define CONFIG_SYS_OR3         (CONFIG_SYS_PRELIM_OR3_AM | \
				       OR_G5LS | \
				       OR_SCY_0_CLK | \
				       OR_BI)

#define CONFIG_SYS_BR3 ((CONFIG_SYS_SPC1920_HPI_BASE & BR_BA_MSK) | \
					       BR_MS_UPMA | \
					       BR_PS_16 | \
					       BR_V)

#define CONFIG_SYS_MAMR (MAMR_GPL_A4DIS | \
		MAMR_RLFA_5X | \
		MAMR_WLFA_5X)

#define CONFIG_SPC1920_HPI_TEST

#ifdef CONFIG_SPC1920_HPI_TEST
#define HPI_REG(x)             (*((volatile u16 *) (CONFIG_SYS_SPC1920_HPI_BASE + x)))
#define HPI_HPIC_1             HPI_REG(0)
#define HPI_HPIC_2             HPI_REG(2)
#define HPI_HPIA_1             HPI_REG(0x2000008)
#define HPI_HPIA_2             HPI_REG(0x2000008 + 2)
#define HPI_HPID_INC_1         HPI_REG(0x1000004)
#define HPI_HPID_INC_2         HPI_REG(0x1000004 + 2)
#define HPI_HPID_NOINC_1       HPI_REG(0x300000c)
#define HPI_HPID_NOINC_2       HPI_REG(0x300000c + 2)
#endif /* CONFIG_SPC1920_HPI_TEST */

/*
 * Ramtron FM18L08 FRAM 32KB on CS4
 */
#define CONFIG_SYS_SPC1920_FRAM_BASE	0x80100000
#define CONFIG_SYS_PRELIM_OR4_AM	0xffff8000
#define CONFIG_SYS_OR4		(CONFIG_SYS_PRELIM_OR4_AM | \
					OR_ACS_DIV2 | \
					OR_BI | \
					OR_SCY_4_CLK | \
					OR_TRLX)

#define CONFIG_SYS_BR4 ((CONFIG_SYS_SPC1920_FRAM_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)

/*
 * PLD CS5
 */
#define CONFIG_SYS_SPC1920_PLD_BASE	0x80000000
#define CONFIG_SYS_PRELIM_OR5_AM	0xffff8000

#define CONFIG_SYS_OR5_PRELIM		(CONFIG_SYS_PRELIM_OR5_AM | \
					OR_CSNT_SAM | \
					OR_ACS_DIV1 | \
					OR_BI | \
					OR_SCY_0_CLK | \
					OR_TRLX)

#define CONFIG_SYS_BR5_PRELIM ((CONFIG_SYS_SPC1920_PLD_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)

#endif	/* __CONFIG_H */
