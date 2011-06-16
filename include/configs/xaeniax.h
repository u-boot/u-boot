/*
 * (C) Copyright 2004-2005
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2004
 * Vincent Dubey, Xa SA, vincent.dubey@xa-ch.com
 *
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.ne
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the xaeniax board.
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
#define CONFIG_PXA250		1	/* This is an PXA255 CPU    */
#define CONFIG_XAENIAX		1	/* on a xaeniax board	    */
#define	CONFIG_SYS_TEXT_BASE	0x0


#define BOARD_LATE_INIT		1


#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_DCACHE_OFF

/*
 * select serial console configuration
 */
#define CONFIG_PXA_SERIAL
#define CONFIG_BTUART	       1       /* we use BTUART on XAENIAX */


/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */

#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 } /* valid baudrates */


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
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

#undef CONFIG_CMD_DTT


#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.68.201
#define CONFIG_SERVERIP		192.168.68.62

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTCOMMAND	"bootm 0x00100000"
#define CONFIG_BOOTARGS		"console=ttyS1,115200"
#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200			/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1			/* which serial port to use */
#endif

/*
 * Size of malloc() pool; this lives below the uppermost 128 KiB which are
 * used for the RAM copy of the uboot code
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 128*1024)

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory	*/
#define CONFIG_SYS_HUSH_PARSER		1

#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"u-boot$ "	/* Monitor Command Prompt */
#else
#define CONFIG_SYS_PROMPT		"u-boot=> "	/* Monitor Command Prompt */
#endif
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_DEVICE_NULLDEV	1

#define CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0xa1000000	/* default load address */

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_CPUSPEED		0x141		/* set core clock to 400/200/100 MHz */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 banks (partition) of DRAM */
#define PHYS_SDRAM_1		0xa0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */
#define PHYS_SDRAM_2		0xa4000000 /* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_3		0xa8000000 /* SDRAM Bank #3 */
#define PHYS_SDRAM_3_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_4		0xac000000 /* SDRAM Bank #4 */
#define PHYS_SDRAM_4_SIZE	0x00000000 /* 0 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0x04000000 /* Flash Bank #2 */
#define PHYS_FLASH_SIZE		0x02000000 /* 32 MB */
#define PHYS_FLASH_BANK_SIZE	0x02000000 /* 32 MB Banks */
#define PHYS_FLASH_SECT_SIZE	0x00040000 /* 256 KB sectors (x2) */

#define CONFIG_SYS_DRAM_BASE		0xa0000000
#define CONFIG_SYS_DRAM_SIZE		0x04000000

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		(GENERATED_GBL_DATA_SIZE + PHYS_SDRAM_1)

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1    /* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	128  /* max number of sectors on one chip    */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* FIXME */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + 0x40000)/* Addr of Environment Sector	*/
#define CONFIG_ENV_SIZE		0x40000			/* Total Size of Environment Sector	*/

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*
 * SMSC91C111 Network Card
 */
#define CONFIG_NET_MULTI
#define CONFIG_SMC91111		1
#define CONFIG_SMC91111_BASE		0x10000300  /* chip select 3         */
#define CONFIG_SMC_USE_32_BIT		1          /* 32 bit bus  */
#undef  CONFIG_SMC_91111_EXT_PHY		   /* we use internal phy   */
#undef  CONFIG_SHOW_ACTIVITY
#define CONFIG_NET_RETRY_COUNT		10	   /* # of retries          */

/*
 * GPIO settings
 */

/*
 * GP05 == nUSBReset  is 1
 * GP10 == CFReset   is 1
 * GP13 == nCFDataEnable is 1
 * GP14 == nCFAddrEnable is 1
 * GP15 == nCS1      is 1
 * GP21 == ComBrdReset is 1
 * GP24 == SFRM      is 1
 * GP25 == TXD       is 1
 * GP31 == SYNC      is 1
 * GP33 == nCS5      is 1
 * GP39 == FFTXD     is 1
 * GP41 == RTS       is 1
 * GP43 == BTTXD     is 1
 * GP45 == BTRTS     is 1
 * GP47 == TXD       is 1
 * GP48 == nPOE      is 1
 * GP49 == nPWE      is 1
 * GP50 == nPIOR     is 1
 * GP51 == nPIOW     is 1
 * GP52 == nPCE[1]   is 1
 * GP53 == nPCE[2]   is 1
 * GP54 == nPSKTSEL  is 1
 * GP55 == nPREG     is 1
 * GP78 == nCS2      is 1
 * GP79 == nCS3      is 1
 * GP80 == nCS4      is 1
 * GP82 == NSSPSFRM  is 1
 * GP83 == NSSPTXD   is 1
 */
#define CONFIG_SYS_GPSR0_VAL		0x8320E420
#define CONFIG_SYS_GPSR1_VAL		0x00FFAA82
#define CONFIG_SYS_GPSR2_VAL		0x000DC000

/*
 * GP03 == LANReset  is 0
 * GP06 == USBWakeUp  is 0
 * GP11 == USBControl is 0
 * GP12 == Buzzer     is 0
 * GP16 == PWM0       is 0
 * GP17 == PWM1       is 0
 * GP23 == SCLK      is 0
 * GP30 == SDATA_OUT is 0
 * GP81 == NSSPCLK   is 0
 */
#define CONFIG_SYS_GPCR0_VAL		0x40C31848
#define CONFIG_SYS_GPCR1_VAL		0x00000000
#define CONFIG_SYS_GPCR2_VAL		0x00020000

/*
 * GP00 == CPUWakeUpUSB is input
 * GP01 == GP reset is input
 * GP02 == LANInterrupt is input
 * GP03 == LANReset     is output
 * GP04 == USBInterrupt is input
 * GP05 == nUSBReset    is output
 * GP06 == USBWakeUp    is output
 * GP07 == CFReady/nBusy is input
 * GP08 == nCFCardDetect1 is input
 * GP09 == nCFCardDetect2 is input
 * GP10 == nCFReset   is output
 * GP11 == USBControl is output
 * GP12 == Buzzer     is output
 * GP13 == CFDataEnable is output
 * GP14 == CFAddressEnable is output
 * GP15 == nCS1      is output
 * GP16 == PWM0      is output
 * GP17 == PWM1      is output
 * GP18 == RDY       is input
 * GP19 == ReaderReady is input
 * GP20 == ReaderReset is input
 * GP21 == ComBrdReset is output
 * GP23 == SCLK      is output
 * GP24 == SFRM      is output
 * GP25 == TXD       is output
 * GP26 == RXD       is input
 * GP27 == EXTCLK    is input
 * GP28 == BITCLK    is output
 * GP29 == SDATA_IN0 is input
 * GP30 == SDATA_OUT is output
 * GP31 == SYNC      is output
 * GP32 == SYSSCLK   is output
 * GP33 == nCS5      is output
 * GP34 == FFRXD     is input
 * GP35 == CTS       is input
 * GP36 == DCD       is input
 * GP37 == DSR       is input
 * GP38 == RI        is input
 * GP39 == FFTXD     is output
 * GP40 == DTR       is output
 * GP41 == RTS       is output
 * GP42 == BTRXD     is input
 * GP43 == BTTXD     is output
 * GP44 == BTCTS     is input
 * GP45 == BTRTS     is output
 * GP46 == RXD       is input
 * GP47 == TXD       is output
 * GP48 == nPOE      is output
 * GP49 == nPWE      is output
 * GP50 == nPIOR     is output
 * GP51 == nPIOW     is output
 * GP52 == nPCE[1]   is output
 * GP53 == nPCE[2]   is output
 * GP54 == nPSKTSEL  is output
 * GP55 == nPREG     is output
 * GP56 == nPWAIT    is input
 * GP57 == nPIOS16   is input
 * GP58 == LDD[0]    is output
 * GP59 == LDD[1]    is output
 * GP60 == LDD[2]    is output
 * GP61 == LDD[3]    is output
 * GP62 == LDD[4]    is output
 * GP63 == LDD[5]    is output
 * GP64 == LDD[6]    is output
 * GP65 == LDD[7]    is output
 * GP66 == LDD[8]    is output
 * GP67 == LDD[9]    is output
 * GP68 == LDD[10]   is output
 * GP69 == LDD[11]   is output
 * GP70 == LDD[12]   is output
 * GP71 == LDD[13]   is output
 * GP72 == LDD[14]   is output
 * GP73 == LDD[15]   is output
 * GP74 == LCD_FCLK  is output
 * GP75 == LCD_LCLK  is output
 * GP76 == LCD_PCLK  is output
 * GP77 == LCD_ACBIAS is output
 * GP78 == nCS2      is output
 * GP79 == nCS3      is output
 * GP80 == nCS4      is output
 * GP81 == NSSPCLK   is output
 * GP82 == NSSPSFRM  is output
 * GP83 == NSSPTXD   is output
 * GP84 == NSSPRXD   is input
 */
#define CONFIG_SYS_GPDR0_VAL		0xD3E3FC68
#define CONFIG_SYS_GPDR1_VAL		0xFCFFAB83
#define CONFIG_SYS_GPDR2_VAL		0x000FFFFF

/*
 * GP01 == GP reset is AF01
 * GP15 == nCS1     is AF10
 * GP16 == PWM0     is AF10
 * GP17 == PWM1     is AF10
 * GP18 == RDY      is AF01
 * GP23 == SCLK     is AF10
 * GP24 == SFRM     is AF10
 * GP25 == TXD      is AF10
 * GP26 == RXD      is AF01
 * GP27 == EXTCLK   is AF01
 * GP28 == BITCLK   is AF01
 * GP29 == SDATA_IN0 is AF10
 * GP30 == SDATA_OUT is AF01
 * GP31 == SYNC     is AF01
 * GP32 == SYSCLK   is AF01
 * GP33 == nCS5  is AF10
 * GP34 == FFRXD is AF01
 * GP35 == CTS   is AF01
 * GP36 == DCD   is AF01
 * GP37 == DSR   is AF01
 * GP38 == RI    is AF01
 * GP39 == FFTXD is AF10
 * GP40 == DTR   is AF10
 * GP41 == RTS   is AF10
 * GP42 == BTRXD is AF01
 * GP43 == BTTXD is AF10
 * GP44 == BTCTS is AF01
 * GP45 == BTRTS is AF10
 * GP46 == RXD   is AF10
 * GP47 == TXD   is AF01
 * GP48 == nPOE  is AF10
 * GP49 == nPWE  is AF10
 * GP50 == nPIOR is AF10
 * GP51 == nPIOW is AF10
 * GP52 == nPCE[1] is AF10
 * GP53 == nPCE[2] is AF10
 * GP54 == nPSKTSEL is AF10
 * GP55 == nPREG   is AF10
 * GP56 == nPWAIT  is AF01
 * GP57 == nPIOS16 is AF01
 * GP58 == LDD[0]  is AF10
 * GP59 == LDD[1]  is AF10
 * GP60 == LDD[2]  is AF10
 * GP61 == LDD[3]  is AF10
 * GP62 == LDD[4]  is AF10
 * GP63 == LDD[5]  is AF10
 * GP64 == LDD[6]  is AF10
 * GP65 == LDD[7]  is AF10
 * GP66 == LDD[8]  is AF10
 * GP67 == LDD[9]  is AF10
 * GP68 == LDD[10] is AF10
 * GP69 == LDD[11] is AF10
 * GP70 == LDD[12] is AF10
 * GP71 == LDD[13] is AF10
 * GP72 == LDD[14] is AF10
 * GP73 == LDD[15] is AF10
 * GP74 == LCD_FCLK is AF10
 * GP75 == LCD_LCLK is AF10
 * GP76 == LCD_PCLK is AF10
 * GP77 == LCD_ACBIAS is AF10
 * GP78 == nCS2     is AF10
 * GP79 == nCS3     is AF10
 * GP80 == nCS4     is AF10
 * GP81 == NSSPCLK  is AF01
 * GP82 == NSSPSFRM is AF01
 * GP83 == NSSPTXD  is AF01
 * GP84 == NSSPRXD  is AF10
 */
#define CONFIG_SYS_GAFR0_L_VAL		0x80000004
#define CONFIG_SYS_GAFR0_U_VAL		0x595A801A
#define CONFIG_SYS_GAFR1_L_VAL		0x699A9559
#define CONFIG_SYS_GAFR1_U_VAL		0xAAA5AAAA
#define CONFIG_SYS_GAFR2_L_VAL		0xAAAAAAAA
#define CONFIG_SYS_GAFR2_U_VAL		0x00000256

/*
 * clock settings
 */
/* RDH = 1
 * PH  = 0
 * VFS = 0
 * BFS = 0
 * SSS = 0
 */
#define CONFIG_SYS_PSSR_VAL		0x00000030

#define CONFIG_SYS_CKEN			0x00000080  /*  */
#define CONFIG_SYS_ICMR			0x00000000  /* No interrupts enabled        */
#define	CONFIG_SYS_CCCR			CCCR_L27|CCCR_M2|CCCR_N10


/*
 * Memory settings
 *
 * This is the configuration for nCS0/1 -> flash banks
 * configuration for nCS1 :
 * [31]    0    -
 * [30:28] 000  -
 * [27:24] 0000 -
 * [23:20] 0000 -
 * [19]    0    -
 * [18:16] 000  -
 * configuration for nCS0:
 * [15]    0    - Slower Device
 * [14:12] 010  - CS deselect to CS time: 2*(2*MemClk) = 40 ns
 * [11:08] 0011 - Address to data valid in bursts: (3+1)*MemClk = 40 ns
 * [07:04] 1111 - " for first access: (23+2)*MemClk = 250 ns (fixme 12+2?)
 * [03]    0    - 32 Bit bus width
 * [02:00] 010  - burst OF 4 ROM or FLASH
*/
#define CONFIG_SYS_MSC0_VAL		0x000023D2

/* This is the configuration for nCS2/3 -> USB controller, LAN
 * configuration for nCS3: LAN
 * [31]    0    - Slower Device
 * [30:28] 001  - RRR3: CS deselect to CS time: 1*(2*MemClk) = 20 ns
 * [27:24] 0010 - RDN3: Address to data valid in bursts: (2+1)*MemClk = 30 ns
 * [23:20] 0010 - RDF3: Address for first access: (2+1)*MemClk = 30 ns
 * [19]    0    - 32 Bit bus width
 * [18:16] 100  - variable latency I/O
 * configuration for nCS2: USB
 * [15]    1    - Faster Device
 * [14:12] 010  - RRR2: CS deselect to CS time: 2*(2*MemClk) = 40 ns
 * [11:08] 0010 - RDN2: Address to data valid in bursts: (2+1)*MemClk = 30 ns
 * [07:04] 0110 - RDF2: Address for first access: (6+1)*MemClk = 70 ns
 * [03]    1    - 16 Bit bus width
 * [02:00] 100  - variable latency I/O
 */
#define CONFIG_SYS_MSC1_VAL		0x1224A26C

/* This is the configuration for nCS4/5 -> LAN
 * configuration for nCS5:
 * [31]    0    -
 * [30:28] 000  -
 * [27:24] 0000 -
 * [23:20] 0000 -
 * [19]    0    -
 * [18:16] 000  -
 * configuration for nCS4: LAN
 * [15]    1    - Faster Device
 * [14:12] 010  - RRR2: CS deselect to CS time: 2*(2*MemClk) = 40 ns
 * [11:08] 0010 - RDN2: Address to data valid in bursts: (2+1)*MemClk = 30 ns
 * [07:04] 0110 - RDF2: Address for first access: (6+1)*MemClk = 70 ns
 * [03]    0    - 32 Bit bus width
 * [02:00] 100  - variable latency I/O
 */
#define CONFIG_SYS_MSC2_VAL		0x00001224

/* MDCNFG: SDRAM Configuration Register
 *
 * [31:29]   000 - reserved
 * [28]      0	 - no SA1111 compatiblity mode
 * [27]      0   - latch return data with return clock
 * [26]      0   - alternate addressing for pair 2/3
 * [25:24]   00  - timings
 * [23]      0   - internal banks in lower partition 2/3 (not used)
 * [22:21]   00  - row address bits for partition 2/3 (not used)
 * [20:19]   00  - column address bits for partition 2/3 (not used)
 * [18]      0   - SDRAM partition 2/3 width is 32 bit
 * [17]      0   - SDRAM partition 3 disabled
 * [16]      0   - SDRAM partition 2 disabled
 * [15:13]   000 - reserved
 * [12]      0	 - no SA1111 compatiblity mode
 * [11]      1   - latch return data with return clock
 * [10]      0   - no alternate addressing for pair 0/1
 * [09:08]   10  - tRP=2*MemClk CL=2 tRCD=2*MemClk tRAS=5*MemClk tRC=8*MemClk
 * [7]       1   - 4 internal banks in lower partition pair
 * [06:05]   10  - 13 row address bits for partition 0/1
 * [04:03]   01  - 9 column address bits for partition 0/1
 * [02]      0   - SDRAM partition 0/1 width is 32 bit
 * [01]      0   - disable SDRAM partition 1
 * [00]      1   - enable  SDRAM partition 0
 */
/* use the configuration above but disable partition 0 */
#define CONFIG_SYS_MDCNFG_VAL		0x00000AC9

/* MDREFR: SDRAM Refresh Control Register
 *
 * [32:26] 0     - reserved
 * [25]    0     - K2FREE: not free running
 * [24]    0     - K1FREE: not free running
 * [23]    0     - K0FREE: not free running
 * [22]    0     - SLFRSH: self refresh disabled
 * [21]    0     - reserved
 * [20]    1     - APD: auto power down
 * [19]    0     - K2DB2: SDCLK2 is MemClk
 * [18]    0     - K2RUN: disable SDCLK2
 * [17]    0     - K1DB2: SDCLK1 is MemClk
 * [16]    1     - K1RUN: enable SDCLK1
 * [15]    1     - E1PIN: SDRAM clock enable
 * [14]    0     - K0DB2: SDCLK0 is MemClk
 * [13]    0     - K0RUN: disable SDCLK0
 * [12]    0     - E0PIN: disable SDCKE0
 * [11:00] 000000011000 - (64ms/8192)*MemClkFreq/32 = 24
 */
#define CONFIG_SYS_MDREFR_VAL		0x00138018 /* mh: was 0x00118018 */

/* MDMRS: Mode Register Set Configuration Register
 *
 * [31]      0       - reserved
 * [30:23]   00000000- MDMRS2: SDRAM2/3 MRS Value. (not used)
 * [22:20]   011     - MDCL2:  SDRAM2/3 Cas Latency.  (not used)
 * [19]      0       - MDADD2: SDRAM2/3 burst Type. Fixed to sequential.  (not used)
 * [18:16]   010     - MDBL2:  SDRAM2/3 burst Length. Fixed to 4.  (not used)
 * [15]      0       - reserved
 * [14:07]   00000000- MDMRS0: SDRAM0/1 MRS Value.
 * [06:04]   011     - MDCL0:  SDRAM0/1 Cas Latency.
 * [03]      0       - MDADD0: SDRAM0/1 burst Type. Fixed to sequential.
 * [02:00]   010     - MDBL0:  SDRAM0/1 burst Length. Fixed to 4.
 */
#define CONFIG_SYS_MDMRS_VAL		0x00320032

#define	CONFIG_SYS_FLYCNFG_VAL		0x00000000
#define	CONFIG_SYS_SXCNFG_VAL		0x00000000

/*
 * PCMCIA and CF Interfaces
 */
#define CONFIG_SYS_MECR_VAL		0x00000000
#define CONFIG_SYS_MCMEM0_VAL		0x00010504
#define CONFIG_SYS_MCMEM1_VAL		0x00010504
#define CONFIG_SYS_MCATT0_VAL		0x00010504
#define CONFIG_SYS_MCATT1_VAL		0x00010504
#define CONFIG_SYS_MCIO0_VAL		0x00004715
#define CONFIG_SYS_MCIO1_VAL		0x00004715


#endif	/* __CONFIG_H */
