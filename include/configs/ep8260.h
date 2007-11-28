/*
 * (C) Copyright 2002
 * Frank Panno <fpanno@delphintech.com>, Delphin Technology AG
 *
 * This file is based on similar values for other boards found in other
 * U-Boot config files, and some that I found in the EP8260 manual.
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
 *
 * "EP8260 H, V.1.1"
 * 	- 64M 60x Bus SDRAM
 * 	- 32M Local Bus SDRAM
 * 	- 16M Flash (4 x AM29DL323DB90WDI)
 * 	- 128k NVRAM with RTC
 *
 * "EP8260 H2, V.1.3" (CFG_EP8260_H2)
 * 	- 300MHz/133MHz/66MHz
 * 	- 64M 60x Bus SDRAM
 * 	- 32M Local Bus SDRAM
 * 	- 32M Flash
 * 	- 128k NVRAM with RTC
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Define this to enable support the EP8260 H2 version */
#define CFG_EP8260_H2	1
/* #undef CFG_EP8260_H2  */

#define CONFIG_CPM2		1	/* Has a CPM2 */

/* What is the oscillator's (UX2) frequency in Hz? */
#define CONFIG_8260_CLKIN  (66 * 1000 * 1000)

/*-----------------------------------------------------------------------
 * MODCK_H & MODCLK[1-3] - Ref: Section 9.2 in MPC8206 User Manual
 *-----------------------------------------------------------------------
 * What should MODCK_H be? It is dependent on the oscillator
 * frequency, MODCK[1-3], and desired CPM and core frequencies.
 * Here are some example values (all frequencies are in MHz):
 *
 * MODCK_H   MODCK[1-3]	 Osc	CPM    Core
 * -------   ----------	 ---	---    ----
 * 0x2	     0x2	 33	133    133
 * 0x2	     0x3	 33	133    166
 * 0x2	     0x4	 33	133    200
 * 0x2	     0x5	 33	133    233
 * 0x2	     0x6	 33	133    266
 *
 * 0x5	     0x5	 66	133    133
 * 0x5	     0x6	 66	133    166
 * 0x5	     0x7	 66	133    200 *
 * 0x6	     0x0	 66	133    233
 * 0x6	     0x1	 66	133    266
 * 0x6	     0x2	 66	133    300
 */
#ifdef CFG_EP8260_H2
#define CFG_SBC_MODCK_H  (HRCW_MODCK_H0110)
#else
#define CFG_SBC_MODCK_H  (HRCW_MODCK_H0110)
#endif

/* Define this if you want to boot from 0x00000100. If you don't define
 * this, you will need to program the bootloader to 0xfff00000, and
 * get the hardware reset config words at 0xfe000000. The simplest
 * way to do that is to program the bootloader at both addresses.
 * It is suggested that you just let U-Boot live at 0x00000000.
 */
/* #define CFG_SBC_BOOT_LOW 1 */	/* only for HRCW */
/* #undef CFG_SBC_BOOT_LOW */

/* The reset command will not work as expected if the reset address does
 * not point to the correct address.
 */

#define CFG_RESET_ADDRESS	0xFFF00100

/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain TEXT_BASE from board/ep8260/config.mk
 * The main FLASH is whichever is connected to *CS0. U-Boot expects
 * this to be the SIMM.
 */
#ifdef CFG_EP8260_H2
#define CFG_FLASH0_BASE 0xFE000000
#define CFG_FLASH0_SIZE 32
#else
#define CFG_FLASH0_BASE 0xFF000000
#define CFG_FLASH0_SIZE 16
#endif

/* What should the base address of the secondary FLASH be and how big
 * is it (in Mbytes)? The secondary FLASH is whichever is connected
 * to *CS6. U-Boot expects this to be the on board FLASH. If you don't
 * want it enabled, don't define these constants.
 */
#define CFG_FLASH1_BASE 0
#define CFG_FLASH1_SIZE 0
#undef CFG_FLASH1_BASE
#undef CFG_FLASH1_SIZE

/* What should be the base address of SDRAM DIMM (60x bus) and how big is
 * it (in Mbytes)?
*/
#define CFG_SDRAM0_BASE 0x00000000
#define CFG_SDRAM0_SIZE 64

/* define CFG_LSDRAM if you want to enable the 32M SDRAM on the
 * local bus (8260 local bus is NOT cacheable!)
*/
/* #define CFG_LSDRAM */
#undef CFG_LSDRAM

#ifdef CFG_LSDRAM
/* What should be the base address of SDRAM DIMM (local bus) and how big is
 * it (in Mbytes)?
*/
  #define CFG_SDRAM1_BASE 0x04000000
  #define CFG_SDRAM1_SIZE 32
#else
  #define CFG_SDRAM1_BASE 0
  #define CFG_SDRAM1_SIZE 0
  #undef CFG_SDRAM1_BASE
  #undef CFG_SDRAM1_SIZE
#endif /* CFG_LSDRAM */

/* What should be the base address of NVRAM and how big is
 * it (in Bytes)
 */
#define CFG_NVRAM_BASE_ADDR  0xFA080000
#define CFG_NVRAM_SIZE       (128*1024)-16

/* The RTC is a Dallas DS1556
 */
#define CONFIG_RTC_DS1556

/* What should be the base address of the LEDs and switch S0?
 * If you don't want them enabled, don't define this.
 */
#define CFG_LED_BASE 0x00000000
#undef CFG_LED_BASE

/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere.
 */
#define CONFIG_CONS_ON_SMC          /* define if console on SMC */
#undef  CONFIG_CONS_ON_SCC          /* define if console on SCC */
#undef  CONFIG_CONS_NONE            /* define if console on neither */
#define CONFIG_CONS_INDEX    1      /* which SMC/SCC channel for console */

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */
#undef  CONFIG_ETHER_ON_SCC           /* define if ethernet on SCC    */
#define CONFIG_ETHER_ON_FCC           /* define if ethernet on FCC    */
#undef  CONFIG_ETHER_NONE             /* define if ethernet on neither */
#define CONFIG_ETHER_INDEX      3     /* which SCC/FCC channel for ethernet */

#if ( CONFIG_ETHER_INDEX == 3 )

/*
 * - Rx-CLK is CLK15
 * - Tx-CLK is CLK16
 * - RAM for BD/Buffers is on the local Bus (see 28-13)
 * - Enable Half Duplex in FSMR
 */
# define CFG_CMXFCR_MASK	(CMXFCR_FC3|CMXFCR_RF3CS_MSK|CMXFCR_TF3CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF3CS_CLK15|CMXFCR_TF3CS_CLK16)

/*
 * - RAM for BD/Buffers is on the local Bus (see 28-13)
 */
#ifdef CFG_LSDRAM
  #define CFG_CPMFCR_RAMTYPE	3
#else /* CFG_LSDRAM */
  #define CFG_CPMFCR_RAMTYPE	0
#endif /* CFG_LSDRAM */

/* - Enable Half Duplex in FSMR */
/* # define CFG_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB) */
# define CFG_FCC_PSMR		0

#else /* CONFIG_ETHER_INDEX */
# error "on EP8260 ethernet must be FCC3"
#endif /* CONFIG_ETHER_INDEX */

/*
 * select i2c support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#undef  CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

/*
 * Software (bit-bang) I2C driver configuration
 */
#ifdef CONFIG_SOFT_I2C
#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00010000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00010000)
#define I2C_READ	((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00010000; \
			else    iop->pdat &= ~0x00010000
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00020000; \
			else    iop->pdat &= ~0x00020000
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */
#endif /* CONFIG_SOFT_I2C */

/* #define CONFIG_RTC_DS174x */

/* Define this to reserve an entire FLASH sector (256 KB) for
 * environment variables. Otherwise, the environment will be
 * put in the same sector as U-Boot, and changing variables
 * will erase U-Boot temporarily
 */
#define CFG_ENV_IN_OWN_SECT

/* Define to allow the user to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* What should the console's baud rate be? */
#ifdef CFG_EP8260_H2
#define CONFIG_BAUDRATE         9600
#else
#define CONFIG_BAUDRATE         115200
#endif

/* Ethernet MAC address */
#define CONFIG_ETHADDR          00:10:EC:00:30:8C

#define CONFIG_IPADDR		192.168.254.130
#define CONFIG_SERVERIP         192.168.254.49

/* Set to a positive value to delay for running BOOTCOMMAND */
#define CONFIG_BOOTDELAY        -1

/* undef this to save memory */
#define CFG_LONGHELP

/* Monitor Command Prompt       */
#define CFG_PROMPT              "=> "

/* Define this variable to enable the "hush" shell (from
   Busybox) as command line interpreter, thus enabling
   powerful command line syntax like
   if...then...else...fi conditionals or `&&' and '||'
   constructs ("shell scripts").
   If undefined, you get the old, much simpler behaviour
   with a somewhat smapper memory footprint.
*/
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "


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
#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CDP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_PING
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

#undef CONFIG_CMD_DCR
#undef CONFIG_CMD_XIMG

/* Where do the internal registers live? */
#define CFG_IMMR               0xF0000000
#define CFG_DEFAULT_IMMR       0x00010000

/* Where do the on board registers (CS4) live? */
#define CFG_REGS_BASE          0xFA000000

/*****************************************************************************
 *
 * You should not have to modify any of the following settings
 *
 *****************************************************************************/

#define CONFIG_MPC8260          1       /* This is an MPC8260 CPU   */
#define CONFIG_EP8260           11      /* on an Embedded Planet EP8260 Board, Rev. 11 */

#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_early_init_f	*/

/*
 * Miscellaneous configurable options
 */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CBSIZE              1024       /* Console I/O Buffer Size      */
#else
#  define CFG_CBSIZE              256        /* Console I/O Buffer Size      */
#endif

/* Print Buffer Size */
#define CFG_PBSIZE        (CFG_CBSIZE + sizeof(CFG_PROMPT)+16)

#define CFG_MAXARGS       8            /* max number of command args   */

#define CFG_BARGSIZE      CFG_CBSIZE   /* Boot Argument Buffer Size    */

#ifdef CFG_LSDRAM
  #define CFG_MEMTEST_START 0x04000000   /* memtest works on  */
  #define CFG_MEMTEST_END   0x06000000   /* 64-96 MB in SDRAM */
#else
  #define CFG_MEMTEST_START 0x00000000   /* memtest works on  */
  #define CFG_MEMTEST_END   0x02000000   /* 0-32 MB in SDRAM */
#endif /* CFG_LSDRAM */

#define	CONFIG_CLOCKS_IN_MHZ	1      /* clocks passsed to Linux in MHz */

#define CFG_LOAD_ADDR     0x00100000   /* default load address */
#define CFG_TFTP_LOADADDR 0x00100000   /* default load address for network file downloads */

#define CFG_HZ            1000         /* decrementer freq: 1 ms ticks */

/* valid baudrates */
#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_FLASH_BASE    CFG_FLASH0_BASE
#define CFG_SDRAM_BASE    CFG_SDRAM0_BASE

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 */

#if defined(CFG_SBC_BOOT_LOW)
#  define  CFG_SBC_HRCW_BOOT_FLAGS  (HRCW_CIP | HRCW_BMS)
#else
#  define  CFG_SBC_HRCW_BOOT_FLAGS  (0x00000000)
#endif /* defined(CFG_SBC_BOOT_LOW) */

#ifdef CFG_EP8260_H2
/* get the HRCW ISB field from CFG_DEFAULT_IMMR */
#define CFG_SBC_HRCW_IMMR ( ((CFG_DEFAULT_IMMR & 0x10000000) >> 10) |\
			    ((CFG_DEFAULT_IMMR & 0x01000000) >> 7)  |\
			    ((CFG_DEFAULT_IMMR & 0x00100000) >> 4) )

#define CFG_HRCW_MASTER (HRCW_EBM                |\
			 HRCW_L2CPC01            |\
			 CFG_SBC_HRCW_IMMR       |\
			 HRCW_APPC10             |\
			 HRCW_CS10PC01           |\
			 CFG_SBC_MODCK_H 	 |\
			 CFG_SBC_HRCW_BOOT_FLAGS)
#else
#define CFG_HRCW_MASTER 0x10400245
#endif

/* no slaves */
#define CFG_HRCW_SLAVE1 0
#define CFG_HRCW_SLAVE2 0
#define CFG_HRCW_SLAVE3 0
#define CFG_HRCW_SLAVE4 0
#define CFG_HRCW_SLAVE5 0
#define CFG_HRCW_SLAVE6 0
#define CFG_HRCW_SLAVE7 0

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR       CFG_IMMR
#define CFG_INIT_RAM_END        0x4000  /* End of used area in DPRAM    */
#define CFG_GBL_DATA_SIZE      128     /* bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 * Note also that the logic that sets CFG_RAMBOOT is platform dependent.
 */
#define CFG_MONITOR_BASE          TEXT_BASE


#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#  define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN      (256 << 10)     /* Reserve 256 kB for Monitor   */
#define CFG_MALLOC_LEN       (128 << 10)     /* Reserve 128 kB for malloc()  */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ        (8 << 20)       /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS   1       /* max number of memory banks         */
#ifdef CFG_EP8260_H2
#define CFG_MAX_FLASH_SECT    128      /* max number of sectors on one chip  */
#else
#define CFG_MAX_FLASH_SECT    71      /* max number of sectors on one chip  */
#endif

#ifdef CFG_EP8260_H2
#define CFG_FLASH_ERASE_TOUT  240000  /* Timeout for Flash Erase (in ms)    */
#define CFG_FLASH_WRITE_TOUT  500     /* Timeout for Flash Write (in ms)    */
#else
#define CFG_FLASH_ERASE_TOUT  8000    /* Timeout for Flash Erase (in ms)    */
#define CFG_FLASH_WRITE_TOUT  1       /* Timeout for Flash Write (in ms)    */
#endif

#ifndef CFG_RAMBOOT
#  define CFG_ENV_IS_IN_FLASH  1

#  ifdef CFG_ENV_IN_OWN_SECT
#    define CFG_ENV_ADDR       (CFG_MONITOR_BASE + 0x40000)
#    define CFG_ENV_SECT_SIZE  0x40000
#  else
#    define CFG_ENV_ADDR (CFG_FLASH_BASE + CFG_MONITOR_LEN - CFG_ENV_SECT_SIZE)
#    define CFG_ENV_SIZE       0x1000  /* Total Size of Environment Sector */
#    define CFG_ENV_SECT_SIZE  0x10000 /* see README - env sect real size */
#  endif /* CFG_ENV_IN_OWN_SECT */
#else
#  define CFG_ENV_IS_IN_NVRAM  1
#  define CFG_ENV_ADDR         (CFG_MONITOR_BASE - 0x1000)
#  define CFG_ENV_SIZE         0x200
#endif /* CFG_RAMBOOT */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE      32      /* For MPC8260 CPU */

#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT     5     /* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers                    2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT   (HID0_ICE  |\
			 HID0_DCE  |\
			 HID0_ICFI |\
			 HID0_DCI  |\
			 HID0_IFEM |\
			 HID0_ABE)
#ifdef CFG_LSDRAM
/* 8260 local bus is NOT cacheable */
#define CFG_HID0_FINAL  (/*HID0_ICE  |*/\
			 HID0_IFEM |\
			 HID0_ABE  |\
			 HID0_EMCP)
#else /* !CFG_LSDRAM */
#define CFG_HID0_FINAL  (HID0_ICE  |\
			 HID0_IFEM |\
			 HID0_ABE  |\
			 HID0_EMCP)
#endif /* CFG_LSDRAM */

#define CFG_HID2        0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register
 *-----------------------------------------------------------------------
 */
#define CFG_RMR         0

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration                                       4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR         (BCR_EBM   |\
			 BCR_PLDP  |\
			 BCR_EAV   |\
			 BCR_NPQM0)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                             4-31
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR      (SIUMCR_L2CPC01 |\
			 SIUMCR_APPC10  |\
			 SIUMCR_CS10PC01)

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control                            11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#ifdef CFG_EP8260_H2
/* TBD: Find out why setting the BMT to 0xff causes the FCC to
 * generate TX buffer underrun errors for large packets under
 * Linux
 */
#define CFG_SYPCR_BMT	0x00000600
#else
#define CFG_SYPCR_BMT	SYPCR_BMT
#endif

#ifdef CFG_LSDRAM
#define CFG_SYPCR       (SYPCR_SWTC |\
			 CFG_SYPCR_BMT  |\
			 SYPCR_PBME |\
			 SYPCR_LBME |\
			 SYPCR_SWP)
#else
#define CFG_SYPCR       (SYPCR_SWTC |\
			 CFG_SYPCR_BMT  |\
			 SYPCR_PBME |\
			 SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control                     4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CFG_TMCNTSC     (TMCNTSC_SEC |\
			 TMCNTSC_ALR |\
			 TMCNTSC_TCF |\
			 TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control                 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#ifdef CFG_EP8260_H2
#define CFG_PISCR       (PISCR_PS  |\
			 PISCR_PTF |\
			 PISCR_PTE)
#else
#define CFG_PISCR	0
#endif

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control                                   9-8
 *-----------------------------------------------------------------------
 */
#ifdef CFG_EP8260_H2
#define CFG_SCCR        (SCCR_DFBRG00)
#else
#define CFG_SCCR        (SCCR_DFBRG01)
#endif

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR        0

/*-----------------------------------------------------------------------
 * MPTPR - Memory Refresh Timer Prescale Register               10-32
 *-----------------------------------------------------------------------
 */
#define CFG_MPTPR	(0x0A00 & MPTPR_PTP_MSK)

/*
 * Init Memory Controller:
 *
 * Bank Bus     Machine PortSz  Device
 * ---- ---     ------- ------  ------
 *  0   60x     GPCM    64 bit  FLASH (BGA - 16MB AMD AM29DL323DB90WDI)
 *  1   60x     SDRAM   64 bit  SDRAM (BGA - 64MB Micron 48LC8M16A2TG)
 *  2   Local   SDRAM   32 bit  SDRAM (BGA - 32MB Micron 48LC8M16A2TG)
 *  3   unused
 *  4   60x     GPCM     8 bit  Board Regs, NVRTC
 *  5   unused
 *  6   unused
 *  7   unused
 *  8   PCMCIA
 *  9   unused
 * 10   unused
 * 11   unused
*/

/*-----------------------------------------------------------------------
 * BRx - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * ORx - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */

/* Bank 0 - FLASH
 *
 */
#define CFG_BR0_PRELIM  ((CFG_FLASH0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64                      |\
			 BRx_DECC_NONE                  |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CFG_OR0_PRELIM  (MEG_TO_AM(CFG_FLASH0_SIZE)     |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_8_CLK                 |\
			 ORxG_EHTR)

/* Bank 1 - SDRAM
 * PSDRAM
 */
#define CFG_BR1_PRELIM  ((CFG_SDRAM0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CFG_OR1_PRELIM  (MEG_TO_AM(CFG_SDRAM0_SIZE)     |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A6             |\
			 ORxS_NUMR_12)

#ifdef CFG_EP8260_H2
#define CFG_PSDMR       0xC34E246E
#else
#define CFG_PSDMR       0xC34E2462
#endif

#define CFG_PSRT	0x64

#ifdef CFG_LSDRAM
/* Bank 2 - SDRAM
 * LSDRAM
 */

  #define CFG_BR2_PRELIM  ((CFG_SDRAM1_BASE & BRx_BA_MSK) |\
			   BRx_PS_32                      |\
			   BRx_MS_SDRAM_L                 |\
			   BRx_V)

  #define CFG_OR2_PRELIM  (MEG_TO_AM(CFG_SDRAM1_SIZE)     |\
			   ORxS_BPD_4                     |\
			   ORxS_ROWST_PBI0_A9             |\
			   ORxS_NUMR_12)

  #define CFG_LSDMR      0x416A2562
  #define CFG_LSRT	0x64
#else
  #define CFG_LSRT	0x0
#endif /* CFG_LSDRAM */

/* Bank 4 - On board registers
 * NVRTC and BCSR
 */
#define CFG_BR4_PRELIM   ((CFG_REGS_BASE & BRx_BA_MSK)  |\
			   BRx_PS_8                     |\
			   BRx_MS_GPCM_P                |\
			   BRx_V)
/*
#define CFG_OR4_PRELIM    (ORxG_AM_MSK                 |\
			   ORxG_CSNT                   |\
			   ORxG_ACS_DIV1               |\
			   ORxG_SCY_10_CLK              |\
			   ORxG_TRLX)
*/
#define CFG_OR4_PRELIM 0xfff00854

#ifdef _NOT_USED_SINCE_NOT_WORKING_
/* Bank 8 - On board registers
 * PCMCIA (currently not working!)
 */
#define CFG_BR8_PRELIM   ((CFG_REGS_BASE & BRx_BA_MSK)  |\
			   BRx_PS_16                     |\
			   BRx_MS_GPCM_P                |\
			   BRx_V)

#define CFG_OR8_PRELIM    (ORxG_AM_MSK                 |\
			   ORxG_CSNT                   |\
			   ORxG_ACS_DIV1               |\
			   ORxG_SETA                   |\
			   ORxG_SCY_10_CLK)
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD   0x01    /* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM   0x02    /* Software reboot                   */

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif  /* __CONFIG_H */
