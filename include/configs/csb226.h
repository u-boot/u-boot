/*
 * (C) Copyright 2000, 2001, 2002
 * Robert Schwebel, Pengutronix, r.schwebel@pengutronix.de.
 *
 * Configuration for the Cogent CSB226 board. For details see
 * http://www.cogcomp.com/csb_csb226.htm
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
 * include/configs/csb226.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBUG 1

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_PXA250		1	/* This is an PXA250 CPU            */
#define CONFIG_CSB226		1	/* on a CSB226 board                */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff      */
					/* for timer/console/ethernet       */
/*
 * Hardware drivers
 */

/*
 * select serial console configuration
 */
#define CONFIG_FFUART		1	/* we use FFUART on CSB226          */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		19200
#undef  CONFIG_MISC_INIT_R		/* not used yet                     */


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

#define CONFIG_CMD_BDI
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_IMI
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_ENV
#define CONFIG_CMD_RUN
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_CACHE


#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttyS0,19200 ip=192.168.1.10,192.168.1.5,,255,255,255,0,csb root=/dev/nfs, ether=0,0x08000000,eth0"
#define CONFIG_ETHADDR		FF:FF:FF:FF:FF:FF
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.1.56
#define CONFIG_SERVERIP		192.168.1.5
#define CONFIG_BOOTCOMMAND	"bootm 0x40000"
#define CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_CMDLINE_TAG	1

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	19200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */

/*
 * Size of malloc() pool; this lives below the uppermost 128 KiB which are
 * used for the RAM copy of the uboot code
 *
 */
#define CFG_MALLOC_LEN		(128*1024)
#define CFG_GBL_DATA_SIZE	128		/* size in bytes reserved for initial data */

#define CFG_LONGHELP				/* undef to save memory         */
#define CFG_PROMPT		"uboot> "	/* Monitor Command Prompt       */
#define CFG_CBSIZE		128		/* Console I/O Buffer Size      */
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args   */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size    */

#define CFG_MEMTEST_START	0xa0400000      /* memtest works on     */
#define CFG_MEMTEST_END         0xa0800000      /* 4 ... 8 MB in DRAM   */

#undef  CFG_CLKS_IN_HZ          /* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR           0xa3000000	/* default load address */
						/* RS: where is this documented? */
						/* RS: is this where U-Boot is  */
						/* RS: relocated to in RAM?      */

#define CFG_HZ                  3686400         /* incrementer freq: 3.6864 MHz */
						/* RS: the oscillator is actually 3680130?? */
#define CFG_CPUSPEED            0x141           /* set core clock to 200/200/100 MHz */
						/* 0101000001 */
						/*      ^^^^^ Memory Speed 99.53 MHz         */
						/*    ^^      Run Mode Speed = 2x Mem Speed  */
						/* ^^         Turbo Mode Sp. = 1x Run M. Sp. */

#define CFG_MONITOR_LEN		0x20000		/* 128 KiB */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * Network chip
 */
#define CONFIG_DRIVER_CS8900	1
#define CS8900_BUS32		1
#define CS8900_BASE		0x08000000

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE        (128*1024)      /* regular stack */
#ifdef  CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    (4*1024)        /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ    (4*1024)        /* FIQ stack */
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM   */
#define PHYS_SDRAM_1		0xa0000000	/* SDRAM Bank #1            */
#define PHYS_SDRAM_1_SIZE	0x02000000	/* 32 MB                    */

#define PHYS_FLASH_1		0x00000000	/* Flash Bank #1            */
#define PHYS_FLASH_SIZE		0x02000000	/* 32 MB                    */

#define CFG_DRAM_BASE		0xa0000000	/* RAM starts here          */
#define CFG_DRAM_SIZE		0x02000000

#define CFG_FLASH_BASE          PHYS_FLASH_1

# if 0
/* FIXME: switch to _documented_ registers */
/*
 * GPIO settings
 *
 * GP15 == nCS1      is 1
 * GP24 == SFRM      is 1
 * GP25 == TXD       is 1
 * GP33 == nCS5      is 1
 * GP39 == FFTXD     is 1
 * GP41 == RTS       is 1
 * GP47 == TXD       is 1
 * GP49 == nPWE      is 1
 * GP62 == LED_B     is 1
 * GP63 == TDM_OE    is 1
 * GP78 == nCS2      is 1
 * GP79 == nCS3      is 1
 * GP80 == nCS4      is 1
 */
#define CFG_GPSR0_VAL       0x03008000
#define CFG_GPSR1_VAL       0xC0028282
#define CFG_GPSR2_VAL       0x0001C000

/* GP02 == DON_RST   is 0
 * GP23 == SCLK      is 0
 * GP45 == USB_ACT   is 0
 * GP60 == PLLEN     is 0
 * GP61 == LED_A     is 0
 * GP73 == SWUPD_LED is 0
 */
#define CFG_GPCR0_VAL       0x00800004
#define CFG_GPCR1_VAL       0x30002000
#define CFG_GPCR2_VAL       0x00000100

/* GP00 == DON_READY is input
 * GP01 == DON_OK    is input
 * GP02 == DON_RST   is output
 * GP03 == RESET_IND is input
 * GP07 == RES11     is input
 * GP09 == RES12     is input
 * GP11 == SWUPDATE  is input
 * GP14 == nPOWEROK  is input
 * GP15 == nCS1      is output
 * GP17 == RES22     is input
 * GP18 == RDY       is input
 * GP23 == SCLK      is output
 * GP24 == SFRM      is output
 * GP25 == TXD       is output
 * GP26 == RXD       is input
 * GP32 == RES21     is input
 * GP33 == nCS5      is output
 * GP34 == FFRXD     is input
 * GP35 == CTS       is input
 * GP39 == FFTXD     is output
 * GP41 == RTS       is output
 * GP42 == USB_OK    is input
 * GP45 == USB_ACT   is output
 * GP46 == RXD       is input
 * GP47 == TXD       is output
 * GP49 == nPWE      is output
 * GP58 == nCPUBUSINT is input
 * GP59 == LANINT    is input
 * GP60 == PLLEN     is output
 * GP61 == LED_A     is output
 * GP62 == LED_B     is output
 * GP63 == TDM_OE    is output
 * GP64 == nDSPINT   is input
 * GP65 == STRAP0    is input
 * GP67 == STRAP1    is input
 * GP69 == STRAP2    is input
 * GP70 == STRAP3    is input
 * GP71 == STRAP4    is input
 * GP73 == SWUPD_LED is output
 * GP78 == nCS2      is output
 * GP79 == nCS3      is output
 * GP80 == nCS4      is output
 */
#define CFG_GPDR0_VAL       0x03808004
#define CFG_GPDR1_VAL       0xF002A282
#define CFG_GPDR2_VAL       0x0001C200

/* GP15 == nCS1  is AF10
 * GP18 == RDY   is AF01
 * GP23 == SCLK  is AF10
 * GP24 == SFRM  is AF10
 * GP25 == TXD   is AF10
 * GP26 == RXD   is AF01
 * GP33 == nCS5  is AF10
 * GP34 == FFRXD is AF01
 * GP35 == CTS   is AF01
 * GP39 == FFTXD is AF10
 * GP41 == RTS   is AF10
 * GP46 == RXD   is AF10
 * GP47 == TXD   is AF01
 * GP49 == nPWE  is AF10
 * GP78 == nCS2  is AF10
 * GP79 == nCS3  is AF10
 * GP80 == nCS4  is AF10
 */
#define CFG_GAFR0_L_VAL     0x80000000
#define CFG_GAFR0_U_VAL     0x001A8010
#define CFG_GAFR1_L_VAL     0x60088058
#define CFG_GAFR1_U_VAL     0x00000008
#define CFG_GAFR2_L_VAL     0xA0000000
#define CFG_GAFR2_U_VAL     0x00000002


/* FIXME: set GPIO_RER/FER */

/* RDH = 1
 * PH  = 1
 * VFS = 1
 * BFS = 1
 * SSS = 1
 */
#define CFG_PSSR_VAL		0x37

/*
 * Memory settings
 *
 * This is the configuration for nCS0/1 -> flash banks
 * configuration for nCS1:
 * [31]    0    - Slower Device
 * [30:28] 010  - CS deselect to CS time: 2*(2*MemClk) = 40 ns
 * [27:24] 0101 - Address to data valid in bursts: (5+1)*MemClk = 60 ns
 * [23:20] 1011 - " for first access: (11+2)*MemClk = 130 ns
 * [19]    1    - 16 Bit bus width
 * [18:16] 000  - nonburst RAM or FLASH
 * configuration for nCS0:
 * [15]    0    - Slower Device
 * [14:12] 010  - CS deselect to CS time: 2*(2*MemClk) = 40 ns
 * [11:08] 0101 - Address to data valid in bursts: (5+1)*MemClk = 60 ns
 * [07:04] 1011 - " for first access: (11+2)*MemClk = 130 ns
 * [03]    1    - 16 Bit bus width
 * [02:00] 000  - nonburst RAM or FLASH
 */
#define CFG_MSC0_VAL		0x25b825b8 /* flash banks                   */

/* This is the configuration for nCS2/3 -> TDM-Switch, DSP
 * configuration for nCS3: DSP
 * [31]    0    - Slower Device
 * [30:28] 001  - RRR3: CS deselect to CS time: 1*(2*MemClk) = 20 ns
 * [27:24] 0010 - RDN3: Address to data valid in bursts: (2+1)*MemClk = 30 ns
 * [23:20] 0011 - RDF3: Address for first access: (3+1)*MemClk = 40 ns
 * [19]    1    - 16 Bit bus width
 * [18:16] 100  - variable latency I/O
 * configuration for nCS2: TDM-Switch
 * [15]    0    - Slower Device
 * [14:12] 101  - RRR2: CS deselect to CS time: 5*(2*MemClk) = 100 ns
 * [11:08] 1001 - RDN2: Address to data valid in bursts: (9+1)*MemClk = 100 ns
 * [07:04] 0011 - RDF2: Address for first access: (3+1)*MemClk = 40 ns
 * [03]    1    - 16 Bit bus width
 * [02:00] 100  - variable latency I/O
 */
#define CFG_MSC1_VAL		0x123C593C /* TDM switch, DSP               */

/* This is the configuration for nCS4/5 -> ExtBus, LAN Controller
 *
 * configuration for nCS5: LAN Controller
 * [31]    0    - Slower Device
 * [30:28] 001  - RRR5: CS deselect to CS time: 1*(2*MemClk) = 20 ns
 * [27:24] 0010 - RDN5: Address to data valid in bursts: (2+1)*MemClk = 30 ns
 * [23:20] 0011 - RDF5: Address for first access: (3+1)*MemClk = 40 ns
 * [19]    1    - 16 Bit bus width
 * [18:16] 100  - variable latency I/O
 * configuration for nCS4: ExtBus
 * [15]    0    - Slower Device
 * [14:12] 110  - RRR4: CS deselect to CS time: 6*(2*MemClk) = 120 ns
 * [11:08] 1100 - RDN4: Address to data valid in bursts: (12+1)*MemClk = 130 ns
 * [07:04] 1101 - RDF4: Address for first access: 13->(15+1)*MemClk = 160 ns
 * [03]    1    - 16 Bit bus width
 * [02:00] 100  - variable latency I/O
 */
#define CFG_MSC2_VAL		0x123C6CDC /* extra bus, LAN controller     */

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
 * [12]      1	 - SA1111 compatiblity mode
 * [11]      1   - latch return data with return clock
 * [10]      0   - no alternate addressing for pair 0/1
 * [09:08]   01  - tRP=2*MemClk CL=2 tRCD=2*MemClk tRAS=5*MemClk tRC=8*MemClk
 * [7]       1   - 4 internal banks in lower partition pair
 * [06:05]   10  - 13 row address bits for partition 0/1
 * [04:03]   01  - 9 column address bits for partition 0/1
 * [02]      0   - SDRAM partition 0/1 width is 32 bit
 * [01]      0   - disable SDRAM partition 1
 * [00]      1   - enable  SDRAM partition 0
 */
/* use the configuration above but disable partition 0 */
#define CFG_MDCNFG_VAL		0x000019c8

/* MDREFR: SDRAM Refresh Control Register
 *
 * [32:26] 0     - reserved
 * [25]    0     - K2FREE: not free running
 * [24]    0     - K1FREE: not free running
 * [23]    1     - K0FREE: not free running
 * [22]    0     - SLFRSH: self refresh disabled
 * [21]    0     - reserved
 * [20]    0     - APD: no auto power down
 * [19]    0     - K2DB2: SDCLK2 is MemClk
 * [18]    0     - K2RUN: disable SDCLK2
 * [17]    0     - K1DB2: SDCLK1 is MemClk
 * [16]    1     - K1RUN: enable SDCLK1
 * [15]    1     - E1PIN: SDRAM clock enable
 * [14]    1     - K0DB2: SDCLK0 is MemClk
 * [13]    0     - K0RUN: disable SDCLK0
 * [12]    1     - E0PIN: disable SDCKE0
 * [11:00] 000000011000 - (64ms/8192)*MemClkFreq/32 = 24
 */
#define CFG_MDREFR_VAL		0x0081D018

/* MDMRS: Mode Register Set Configuration Register
 *
 * [31]      0       - reserved
 * [30:23]   00000000- MDMRS2: SDRAM2/3 MRS Value. (not used)
 * [22:20]   000     - MDCL2:  SDRAM2/3 Cas Latency.  (not used)
 * [19]      0       - MDADD2: SDRAM2/3 burst Type. Fixed to sequential.  (not used)
 * [18:16]   010     - MDBL2:  SDRAM2/3 burst Length. Fixed to 4.  (not used)
 * [15]      0       - reserved
 * [14:07]   00000000- MDMRS0: SDRAM0/1 MRS Value.
 * [06:04]   010     - MDCL0:  SDRAM0/1 Cas Latency.
 * [03]      0       - MDADD0: SDRAM0/1 burst Type. Fixed to sequential.
 * [02:00]   010     - MDBL0:  SDRAM0/1 burst Length. Fixed to 4.
 */
#define CFG_MDMRS_VAL		0x00020022

/*
 * PCMCIA and CF Interfaces
 */
#define CFG_MECR_VAL		0x00000000
#define CFG_MCMEM0_VAL		0x00000000
#define CFG_MCMEM1_VAL		0x00000000
#define CFG_MCATT0_VAL		0x00000000
#define CFG_MCATT1_VAL		0x00000000
#define CFG_MCIO0_VAL		0x00000000
#define CFG_MCIO1_VAL		0x00000000
#endif

/*
 * GPIO settings
 */
#define CFG_GPSR0_VAL		0xFFFFFFFF
#define CFG_GPSR1_VAL		0xFFFFFFFF
#define CFG_GPSR2_VAL		0xFFFFFFFF
#define CFG_GPCR0_VAL		0x08022080
#define CFG_GPCR1_VAL		0x00000000
#define CFG_GPCR2_VAL		0x00000000
#define CFG_GPDR0_VAL		0xCD82A878
#define CFG_GPDR1_VAL		0xFCFFAB80
#define CFG_GPDR2_VAL		0x0001FFFF
#define CFG_GAFR0_L_VAL		0x80000000
#define CFG_GAFR0_U_VAL		0xA5254010
#define CFG_GAFR1_L_VAL		0x599A9550
#define CFG_GAFR1_U_VAL		0xAAA5AAAA
#define CFG_GAFR2_L_VAL		0xAAAAAAAA
#define CFG_GAFR2_U_VAL		0x00000002

/* FIXME: set GPIO_RER/FER */

#define CFG_PSSR_VAL        0x20

/*
 * Memory settings
 */

#define CFG_MSC0_VAL            0x2ef15af0
#define CFG_MSC1_VAL            0x00003ff4
#define CFG_MSC2_VAL            0x7ff07ff0
#define CFG_MDCNFG_VAL          0x09a909a9
#define CFG_MDREFR_VAL          0x038ff030
#define CFG_MDMRS_VAL           0x00220022

/*
 * PCMCIA and CF Interfaces
 */
#define CFG_MECR_VAL        0x00000000
#define CFG_MCMEM0_VAL      0x00000000
#define CFG_MCMEM1_VAL      0x00000000
#define CFG_MCATT0_VAL      0x00000000
#define CFG_MCATT1_VAL      0x00000000
#define CFG_MCIO0_VAL       0x00000000
#define CFG_MCIO1_VAL       0x00000000

#define CSB226_USER_LED0	0x00000008
#define CSB226_USER_LED1	0x00000010
#define CSB226_USER_LED2	0x00000020


/*
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1	/* max number of memory banks       */
#define CFG_MAX_FLASH_SECT	128	/* max number of sect. on one chip  */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT    (2*CFG_HZ) /* Timeout for Flash Erase       */
#define CFG_FLASH_WRITE_TOUT    (2*CFG_HZ) /* Timeout for Flash Write       */

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR            (PHYS_FLASH_1 + 0x1C000)
					/* Addr of Environment Sector       */
#define CFG_ENV_SIZE            0x4000  /* Total Size of Environment Sector */

#endif  /* __CONFIG_H */
