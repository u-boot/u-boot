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
 * If we are developing, we might want to start U-Boot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#define CONFIG_INIT_CRITICAL		/* undef for developing */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_PXA250 		1	/* This is an PXA250 CPU            */
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

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL & ~CFG_CMD_NET)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttyS0,19200 ip=dhcp root=/dev/nfs, ether=0,0x08000000,eth0"
#define CONFIG_ETHADDR		FF:FF:FF:FF:FF:FF
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.1.56
#define CONFIG_SERVERIP		192.168.1.2
#define CONFIG_BOOTCOMMAND	"bootm 0x40000"
#define CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_CMDLINE_TAG	1

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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

/*
 * GPIO settings
 */
#define CFG_GPSR0_VAL       0xFFFFFFFF
#define CFG_GPSR1_VAL       0xFFFFFFFF
#define CFG_GPSR2_VAL       0xFFFFFFFF
#define CFG_GPCR0_VAL       0x08022080
#define CFG_GPCR1_VAL       0x00000000
#define CFG_GPCR2_VAL       0x00000000
#define CFG_GPDR0_VAL       0xCD82A878
#define CFG_GPDR1_VAL       0xFCFFAB80
#define CFG_GPDR2_VAL       0x0001FFFF
#define CFG_GAFR0_L_VAL     0x80000000
#define CFG_GAFR0_U_VAL     0xA5254010
#define CFG_GAFR1_L_VAL     0x599A9550
#define CFG_GAFR1_U_VAL     0xAAA5AAAA
#define CFG_GAFR2_L_VAL     0xAAAAAAAA
#define CFG_GAFR2_U_VAL     0x00000002

/* FIXME: set GPIO_RER/FER */

#define CFG_PSSR_VAL        0x20

/*
 * Memory settings
 */
#define CFG_MSC0_VAL        0x2EF025D0
#define CFG_MSC1_VAL        0x00003F64
#define CFG_MSC2_VAL        0x00000000
#define CFG_MDCNFG_VAL      0x09a909a9
#define CFG_MDREFR_VAL      0x03ca0030
#define CFG_MDMRS_VAL       0x00220022

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
