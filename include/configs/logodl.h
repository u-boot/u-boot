/*
 * (C) Copyright 2003
 * Robert Schwebel, Pengutronix, r.schwebel@pengutronix.de.
 *
 * Configuration for the Logotronic DL board.
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
 * include/configs/logodl.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_PXA250		1	/* This is an PXA250 CPU            */
#define CONFIG_GEALOG		1	/* on a Logotronic GEALOG SG board  */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff      */
					/* for timer/console/ethernet       */

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_NO_DCACHE

/*
 * Hardware drivers
 */

/*
 * select serial console configuration
 */
#define CONFIG_PXA_SERIAL
#define CONFIG_FFUART		1	/* we use FFUART                    */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		19200
#undef CONFIG_MISC_INIT_R		/* FIXME: misc_init_r() missing     */


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
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_RUN


#define CONFIG_BOOTDELAY	3
/* #define CONFIG_BOOTARGS	"root=/dev/nfs ip=bootp console=ttyS0,19200" */
#define CONFIG_BOOTARGS		"console=ttyS0,19200"
#define CONFIG_ETHADDR		FF:FF:FF:FF:FF:FF
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.1.56
#define CONFIG_SERVERIP		192.168.1.2
#define CONFIG_BOOTCOMMAND	"bootm 0x40000"
#define CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_CMDLINE_TAG	1

/*
 * Miscellaneous configurable options
 */

/*
 * Size of malloc() pool; this lives below the uppermost 128 KiB which are
 * used for the RAM copy of the uboot code
 *
 */
#define CONFIG_SYS_MALLOC_LEN		(256*1024)

#define CONFIG_SYS_LONGHELP				/* undef to save memory         */
#define CONFIG_SYS_PROMPT		"uboot> "	/* Monitor Command Prompt       */
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size      */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START	0x08000000      /* memtest works on             */
#define CONFIG_SYS_MEMTEST_END         0x0800ffff	/* 64 KiB                       */

#define CONFIG_SYS_LOAD_ADDR           0x08000000      /* load kernel to this address   */

#define CONFIG_SYS_HZ			1000
						/* RS: the oscillator is actually 3680130?? */

#define CONFIG_SYS_CPUSPEED            0x141           /* set core clock to 200/200/100 MHz */
						/* 0101000001 */
						/*      ^^^^^ Memory Speed 99.53 MHz         */
						/*    ^^      Run Mode Speed = 2x Mem Speed  */
						/* ^^         Turbo Mode Sp. = 1x Run M. Sp. */

#define CONFIG_SYS_MONITOR_LEN		0x20000		/* 128 KiB */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * SMSC91C111 Network Card
 */
#if 0
#define CONFIG_DRIVER_SMC91111		1
#define CONFIG_SMC91111_BASE		0x10000000 /* chip select 4         */
#undef  CONFIG_SMC_USE_32_BIT		           /* 16 bit bus access     */
#undef  CONFIG_SMC_91111_EXT_PHY		   /* we use internal phy   */
#undef  CONFIG_SHOW_ACTIVITY
#define CONFIG_NET_RETRY_COUNT		10	   /* # of retries          */
#endif

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
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of RAM    */
#define PHYS_SDRAM_1		0x08000000	/* SRAM Bank #1             */
#define PHYS_SDRAM_1_SIZE	(4*1024*1024)	/* 4 MB                     */

#define PHYS_FLASH_1		0x00000000	/* Flash Bank #1            */
#define PHYS_FLASH_2		0x01000000	/* Flash Bank #2            */
#define PHYS_FLASH_SIZE		(32*1024*1024)	/* 32 MB                    */

#define CONFIG_SYS_DRAM_BASE		PHYS_SDRAM_1	/* RAM starts here          */
#define CONFIG_SYS_DRAM_SIZE		PHYS_SDRAM_1_SIZE

#define CONFIG_SYS_FLASH_BASE          PHYS_FLASH_1


/*
 * GPIO settings
 *
 * GP?? == FOOBAR    is 0/1
 */

#define _BIT0       0x00000001
#define _BIT1       0x00000002
#define _BIT2       0x00000004
#define _BIT3       0x00000008

#define _BIT4       0x00000010
#define _BIT5       0x00000020
#define _BIT6       0x00000040
#define _BIT7       0x00000080

#define _BIT8       0x00000100
#define _BIT9       0x00000200
#define _BIT10      0x00000400
#define _BIT11      0x00000800

#define _BIT12      0x00001000
#define _BIT13      0x00002000
#define _BIT14      0x00004000
#define _BIT15      0x00008000

#define _BIT16      0x00010000
#define _BIT17      0x00020000
#define _BIT18      0x00040000
#define _BIT19      0x00080000

#define _BIT20      0x00100000
#define _BIT21      0x00200000
#define _BIT22      0x00400000
#define _BIT23      0x00800000

#define _BIT24      0x01000000
#define _BIT25      0x02000000
#define _BIT26      0x04000000
#define _BIT27      0x08000000

#define _BIT28      0x10000000
#define _BIT29      0x20000000
#define _BIT30      0x40000000
#define _BIT31      0x80000000


#define CONFIG_SYS_LED_A_BIT           (_BIT18)
#define CONFIG_SYS_LED_A_SR            GPSR0
#define CONFIG_SYS_LED_A_CR            GPCR0

#define CONFIG_SYS_LED_B_BIT           (_BIT16)
#define CONFIG_SYS_LED_B_SR            GPSR1
#define CONFIG_SYS_LED_B_CR            GPCR1


/* LED A: off, LED B: off */
#define CONFIG_SYS_GPSR0_VAL       (_BIT1+_BIT6+_BIT8+_BIT9+_BIT11+_BIT15+_BIT16+_BIT18)
#define CONFIG_SYS_GPSR1_VAL       (_BIT0+_BIT1+_BIT16+_BIT24+_BIT25  +_BIT7+_BIT8+_BIT9+_BIT11+_BIT13)
#define CONFIG_SYS_GPSR2_VAL       (_BIT14+_BIT15+_BIT16)

#define CONFIG_SYS_GPCR0_VAL       0x00000000
#define CONFIG_SYS_GPCR1_VAL       0x00000000
#define CONFIG_SYS_GPCR2_VAL       0x00000000

#define CONFIG_SYS_GPDR0_VAL       (_BIT1+_BIT6+_BIT8+_BIT9+_BIT11+_BIT15+_BIT16+_BIT17+_BIT18)
#define CONFIG_SYS_GPDR1_VAL       (_BIT0+_BIT1+_BIT16+_BIT24+_BIT25  +_BIT7+_BIT8+_BIT9+_BIT11+_BIT13)
#define CONFIG_SYS_GPDR2_VAL       (_BIT14+_BIT15+_BIT16)

#define CONFIG_SYS_GAFR0_L_VAL     (_BIT22+_BIT24+_BIT31)
#define CONFIG_SYS_GAFR0_U_VAL     (_BIT15+_BIT17+_BIT19+\
			     _BIT20+_BIT22+_BIT24+_BIT26+_BIT29+_BIT31)
#define CONFIG_SYS_GAFR1_L_VAL     (_BIT3+_BIT4+_BIT6+_BIT8+_BIT10+_BIT12+_BIT15+_BIT17+_BIT19+\
			     _BIT20+_BIT23+_BIT24+_BIT27+_BIT28+_BIT31)
#define CONFIG_SYS_GAFR1_U_VAL     (_BIT21+_BIT23+_BIT25+_BIT27+_BIT29+_BIT31)
#define CONFIG_SYS_GAFR2_L_VAL     (_BIT1+_BIT3+_BIT5+_BIT7+_BIT9+_BIT11+_BIT13+_BIT15+_BIT17+\
			     _BIT19+_BIT21+_BIT23+_BIT25+_BIT27+_BIT29+_BIT31)
#define CONFIG_SYS_GAFR2_U_VAL     (_BIT1)

#define CONFIG_SYS_PSSR_VAL        (0x20)

/*
 * Memory settings
 */
#define CONFIG_SYS_MSC0_VAL	0x123c2980
#define CONFIG_SYS_MSC1_VAL	0x123c2661
#define CONFIG_SYS_MSC2_VAL	0x7ff87ff8


/* no sdram/pcmcia here */
#define CONFIG_SYS_MDCNFG_VAL		0x00000000
#define CONFIG_SYS_MDREFR_VAL		0x00000000
#define CONFIG_SYS_MDREFR_VAL_100	0x00000000
#define CONFIG_SYS_MDMRS_VAL		0x00000000

/* only SRAM */
#define SXCNFG_SETTINGS	0x00000000

/*
 * PCMCIA and CF Interfaces
 */

#define CONFIG_SYS_MECR_VAL        0x00000000
#define CONFIG_SYS_MCMEM0_VAL      0x00010504
#define CONFIG_SYS_MCMEM1_VAL      0x00010504
#define CONFIG_SYS_MCATT0_VAL      0x00010504
#define CONFIG_SYS_MCATT1_VAL      0x00010504
#define CONFIG_SYS_MCIO0_VAL       0x00004715
#define CONFIG_SYS_MCIO1_VAL       0x00004715


/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS     2       /* max number of memory banks           */
#define CONFIG_SYS_MAX_FLASH_SECT      128  /* max number of sectors on one chip    */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT    (2*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT    (2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* FIXME */
#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR            (PHYS_FLASH_1 + 0x1C000)        /* Addr of Environment Sector   */
#define CONFIG_ENV_SIZE            0x4000  /* Total Size of Environment Sector     */

#endif  /* __CONFIG_H */
