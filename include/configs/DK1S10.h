/*
 * (C) Copyright 2003, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
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

#ifndef __CONFIG_H
#define __CONFIG_H

/***********************************************************************
 * Include the whole NIOS CPU configuration.
 *
 * !!! HAVE TO BE HERE !!! DON'T MOVE THIS PART !!!
 *
 ***********************************************************************/

#if	defined(CONFIG_NIOS_SAFE_32)
#include <configs/DK1S10_safe_32.h>
#elif	defined(CONFIG_NIOS_STANDARD_32)
#include <configs/DK1S10_standard_32.h>
#elif	defined(CONFIG_NIOS_MTX_LDK_20)
#include <configs/DK1S10_mtx_ldk_20.h>
#else
#error *** CONFIG_SYS_ERROR: you have to setup right NIOS CPU configuration
#endif

/*------------------------------------------------------------------------
 * BOARD/CPU -- TOP-LEVEL
 *----------------------------------------------------------------------*/
#define CONFIG_NIOS		1		/* NIOS-32 core		*/
#define	CONFIG_DK1S10		1		/* Stratix DK-1S10 board*/
#define CONFIG_SYS_CLK_FREQ	CONFIG_SYS_NIOS_CPU_CLK/* 50 MHz core clock	*/
#define	CONFIG_SYS_HZ			1000		/* 1 msec time tick	*/
#define	CONFIG_BOARD_EARLY_INIT_F 1	/* enable early board-spec. init*/

/*------------------------------------------------------------------------
 * BASE ADDRESSES / SIZE (Flash, SRAM, SDRAM)
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_SDRAM_SIZE != 0)

#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_NIOS_CPU_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		CONFIG_SYS_NIOS_CPU_SDRAM_SIZE

#else
#error *** CONFIG_SYS_ERROR: you have to setup any SDRAM in NIOS CPU config
#endif

#if	defined(CONFIG_SYS_NIOS_CPU_SRAM_BASE) && defined(CONFIG_SYS_NIOS_CPU_SRAM_SIZE)

#define	CONFIG_SYS_SRAM_BASE		CONFIG_SYS_NIOS_CPU_SRAM_BASE
#define	CONFIG_SYS_SRAM_SIZE		CONFIG_SYS_NIOS_CPU_SRAM_SIZE

#else

#undef	CONFIG_SYS_SRAM_BASE
#undef	CONFIG_SYS_SRAM_SIZE

#endif

#define CONFIG_SYS_VECT_BASE		CONFIG_SYS_NIOS_CPU_VEC_BASE

/*------------------------------------------------------------------------
 * MEMORY ORGANIZATION - For the most part, you can put things pretty
 * much anywhere. This is pretty flexible for Nios. So here we make some
 * arbitrary choices & assume that the monitor is placed at the end of
 * a memory resource (so you must make sure TEXT_BASE is chosen
 * appropriately).
 *
 *	-The heap is placed below the monitor.
 *	-Global data is placed below the heap.
 *	-The stack is placed below global data (&grows down).
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256k		*/
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* Global data size rsvd*/
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MONITOR_BASE - CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_MALLOC_BASE - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP		CONFIG_SYS_GBL_DATA_OFFSET

/*------------------------------------------------------------------------
 * FLASH (AM29LV065D)
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_FLASH_SIZE != 0)

#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_NIOS_CPU_FLASH_BASE
#define CONFIG_SYS_FLASH_SIZE		CONFIG_SYS_NIOS_CPU_FLASH_SIZE
#define CONFIG_SYS_MAX_FLASH_SECT	128		/* Max # sects per bank */
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* Max # of flash banks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	8000		/* Erase timeout (msec) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	100		/* Write timeout (msec) */
#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned char	/* flash word size	*/

#else
#error *** CONFIG_SYS_ERROR: you have to setup any Flash memory in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * ENVIRONMENT
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_FLASH_SIZE != 0)

#define	CONFIG_ENV_IS_IN_FLASH	1		/* Environment in flash */

#if	defined(CONFIG_NIOS_STANDARD_32)
#define CONFIG_ENV_ADDR		CONFIG_SYS_FLASH_BASE	/* Mem addr of env	*/
#elif	defined(CONFIG_NIOS_MTX_LDK_20)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#else
#error *** CONFIG_SYS_ERROR: you have to setup the environment base address CONFIG_ENV_ADDR
#endif

#define CONFIG_ENV_SIZE		(64 * 1024)	/* 64 KByte (1 sector)	*/
#define CONFIG_ENV_OVERWRITE			/* Serial/eth change Ok */

#else
#define	CONFIG_ENV_IS_NOWHERE	1		/* NO Environment	*/
#endif

/*------------------------------------------------------------------------
 * CONSOLE
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_UART_NUMS != 0)

#define CONFIG_SYS_NIOS_CONSOLE	CONFIG_SYS_NIOS_CPU_UART0 /* 1st UART is Cons. */
#define CONFIG_LOADS_ECHO	1	 /* echo on for serial download */

#if	(CONFIG_SYS_NIOS_CPU_UART0_BR != 0)
#define CONFIG_SYS_NIOS_FIXEDBAUD	1		   /* Baudrate is fixed	*/
#define CONFIG_BAUDRATE		CONFIG_SYS_NIOS_CPU_UART0_BR
#else
#undef	CONFIG_SYS_NIOS_FIXEDBAUD
#define CONFIG_BAUDRATE		115200
#endif

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#else
#error *** CONFIG_SYS_ERROR: you have to setup at least one UART in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * TIMER FOR TIMEBASE -- Nios doesn't have the equivalent of ppc  PIT,
 * so an avalon bus timer is required.
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_TIMER_NUMS != 0) && defined(CONFIG_SYS_NIOS_CPU_TICK_TIMER)

#if	(CONFIG_SYS_NIOS_CPU_TICK_TIMER == 0)

#define CONFIG_SYS_NIOS_TMRBASE	CONFIG_SYS_NIOS_CPU_TIMER0 /* TIMER0 as tick	*/
#define CONFIG_SYS_NIOS_TMRIRQ		CONFIG_SYS_NIOS_CPU_TIMER0_IRQ

#if	(CONFIG_SYS_NIOS_CPU_TIMER0_FP == 1)		    /* fixed period */

#if	(CONFIG_SYS_NIOS_CPU_TIMER0_PER >= CONFIG_SYS_HZ)
#define CONFIG_SYS_NIOS_TMRMS		(CONFIG_SYS_NIOS_CPU_TIMER0_PER / CONFIG_SYS_HZ)
#else
#error *** CONFIG_SYS_ERROR: you have to use a timer periode greater than CONFIG_SYS_HZ
#endif

#undef	CONFIG_SYS_NIOS_TMRCNT	/* no preloadable counter value */

#elif	(CONFIG_SYS_NIOS_CPU_TIMER0_FP == 0)		    /* variable period */

#if	(CONFIG_SYS_HZ <= 1000)
#define CONFIG_SYS_NIOS_TMRMS		(1000 / CONFIG_SYS_HZ)
#else
#error *** CONFIG_SYS_ERROR: sorry, CONFIG_SYS_HZ have to be less than 1000
#endif

#define	CONFIG_SYS_NIOS_TMRCNT		(CONFIG_SYS_CLK_FREQ / CONFIG_SYS_HZ)

#else
#error *** CONFIG_SYS_ERROR: you have to define CONFIG_SYS_NIOS_CPU_TIMER0_FP correct
#endif

#elif	(CONFIG_SYS_NIOS_CPU_TICK_TIMER == 1)

#define CONFIG_SYS_NIOS_TMRBASE	CONFIG_SYS_NIOS_CPU_TIMER1 /* TIMER1 as tick	*/
#define CONFIG_SYS_NIOS_TMRIRQ		CONFIG_SYS_NIOS_CPU_TIMER1_IRQ

#if	(CONFIG_SYS_NIOS_CPU_TIMER1_FP == 1)		    /* fixed period */

#if	(CONFIG_SYS_NIOS_CPU_TIMER1_PER >= CONFIG_SYS_HZ)
#define CONFIG_SYS_NIOS_TMRMS		(CONFIG_SYS_NIOS_CPU_TIMER1_PER / CONFIG_SYS_HZ)
#else
#error *** CONFIG_SYS_ERROR: you have to use a timer periode greater than CONFIG_SYS_HZ
#endif

#undef	CONFIG_SYS_NIOS_TMRCNT	/* no preloadable counter value */

#elif	(CONFIG_SYS_NIOS_CPU_TIMER1_FP == 0)		    /* variable period */

#if	(CONFIG_SYS_HZ <= 1000)
#define CONFIG_SYS_NIOS_TMRMS		(1000 / CONFIG_SYS_HZ)
#else
#error *** CONFIG_SYS_ERROR: sorry, CONFIG_SYS_HZ have to be less than 1000
#endif

#define	CONFIG_SYS_NIOS_TMRCNT		(CONFIG_SYS_CLK_FREQ / CONFIG_SYS_HZ)

#else
#error *** CONFIG_SYS_ERROR: you have to define CONFIG_SYS_NIOS_CPU_TIMER1_FP correct
#endif

#endif	/* CONFIG_SYS_NIOS_CPU_TICK_TIMER */

#else
#error *** CONFIG_SYS_ERROR: you have to setup at least one TIMER in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * Ethernet -- needs work!
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_LAN_NUMS == 1)

#if	(CONFIG_SYS_NIOS_CPU_LAN0_TYPE == 0)		/* LAN91C111		*/

#define	CONFIG_DRIVER_SMC91111			/* Using SMC91c111	*/
#undef	CONFIG_SMC91111_EXT_PHY			/* Internal PHY		*/
#define	CONFIG_SMC91111_BASE	(CONFIG_SYS_NIOS_CPU_LAN0_BASE + CONFIG_SYS_NIOS_CPU_LAN0_OFFS)

#if	(CONFIG_SYS_NIOS_CPU_LAN0_BUSW == 32)
#define	CONFIG_SMC_USE_32_BIT	1
#else	/* no */
#undef	CONFIG_SMC_USE_32_BIT
#endif

#elif	(CONFIG_SYS_NIOS_CPU_LAN0_TYPE == 1)		/* CS8900A		*/

	/********************************************/
	/* !!! CS8900 is __not__ tested on NIOS !!! */
	/********************************************/
#define	CONFIG_DRIVER_CS8900			/* Using CS8900		*/
#define	CS8900_BASE		(CONFIG_SYS_NIOS_CPU_LAN0_BASE + CONFIG_SYS_NIOS_CPU_LAN0_OFFS)

#if	(CONFIG_SYS_NIOS_CPU_LAN0_BUSW == 32)
#undef	CS8900_BUS16
#define	CS8900_BUS32		1
#else	/* no */
#define	CS8900_BUS16		1
#undef	CS8900_BUS32
#endif

#else
#error *** CONFIG_SYS_ERROR: invalid LAN0 chip type, check your NIOS CPU config
#endif

#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.2.21
#define CONFIG_SERVERIP		192.168.2.16

#else
#error *** CONFIG_SYS_ERROR: you have to setup just one LAN only or expand your config.h
#endif

/*------------------------------------------------------------------------
 * STATUS LEDs
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_PIO_NUMS != 0) && defined(CONFIG_SYS_NIOS_CPU_LED_PIO)

#if	(CONFIG_SYS_NIOS_CPU_LED_PIO == 0)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO0 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 1)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO1 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 2)

#define	STATUS_LED_BASE			CONFIG_SYS_NIOS_CPU_PIO2
#define	STATUS_LED_BITS			CONFIG_SYS_NIOS_CPU_PIO2_BITS
#define	STATUS_LED_ACTIVE		1 /* LED on for bit == 1 */

#if	(CONFIG_SYS_NIOS_CPU_PIO2_TYPE == 1)
#define	STATUS_LED_WRONLY		1
#else
#undef	STATUS_LED_WRONLY
#endif

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 3)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO3 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 4)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO4 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 5)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO5 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 6)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO6 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 7)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO7 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 8)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO8 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 9)

#error *** CONFIG_SYS_ERROR: status LEDs at PIO9 not supported, expand your config.h

#else
#error *** CONFIG_SYS_ERROR: you have to set CONFIG_SYS_NIOS_CPU_LED_PIO in right case
#endif

#define	CONFIG_STATUS_LED		1 /* enable status led driver */

#define	STATUS_LED_BIT			(1 << 0)	/* LED[0] */
#define	STATUS_LED_STATE		STATUS_LED_BLINKING
#define	STATUS_LED_BOOT_STATE		STATUS_LED_OFF
#define	STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 10)	/* ca. 1 Hz */
#define	STATUS_LED_BOOT			0		/* boot LED */

#if	(STATUS_LED_BITS > 1)
#define	STATUS_LED_BIT1			(1 << 1)	/* LED[1] */
#define	STATUS_LED_STATE1		STATUS_LED_OFF
#define	STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 50)	/* ca. 5 Hz */
#define	STATUS_LED_RED			1		/* fail LED */
#endif

#if	(STATUS_LED_BITS > 2)
#define	STATUS_LED_BIT2			(1 << 2)	/* LED[2] */
#define	STATUS_LED_STATE2		STATUS_LED_OFF
#define	STATUS_LED_PERIOD2		(CONFIG_SYS_HZ / 10)	/* ca. 1 Hz */
#define	STATUS_LED_YELLOW		2		/* info LED */
#endif

#if	(STATUS_LED_BITS > 3)
#define	STATUS_LED_BIT3			(1 << 3)	/* LED[3] */
#define	STATUS_LED_STATE3		STATUS_LED_OFF
#define	STATUS_LED_PERIOD3		(CONFIG_SYS_HZ / 10)	/* ca. 1 Hz */
#define	STATUS_LED_GREEN		3		/* info LED */
#endif

#define	STATUS_LED_PAR			1 /* makes status_led.h happy */

#endif	/* CONFIG_SYS_NIOS_CPU_PIO_NUMS */

/*------------------------------------------------------------------------
 * SEVEN SEGMENT LED DISPLAY
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_PIO_NUMS != 0) && defined(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO)

#if	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 0)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO0 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 1)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO1 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 2)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO2 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 3)

#define	SEVENSEG_BASE			CONFIG_SYS_NIOS_CPU_PIO3
#define	SEVENSEG_BITS			CONFIG_SYS_NIOS_CPU_PIO3_BITS
#define	SEVENSEG_ACTIVE			0 /* LED on for bit == 1 */

#if	(CONFIG_SYS_NIOS_CPU_PIO3_TYPE == 1)
#define	SEVENSEG_WRONLY			1
#else
#undef	SEVENSEG_WRONLY
#endif

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 4)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO4 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 5)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO5 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 6)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO6 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 7)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO7 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 8)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO8 not supported, expand your config.h

#elif	(CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO == 9)

#error *** CONFIG_SYS_ERROR: seven segment display at PIO9 not supported, expand your config.h

#else
#error *** CONFIG_SYS_ERROR: you have to set CONFIG_SYS_NIOS_CPU_SEVENSEG_PIO in right case
#endif

#define	CONFIG_SEVENSEG			1 /* enable seven segment led driver */

/*
 * Dual 7-Segment Display pin assignment -- read more in your
 * "Nios Development Board Reference Manual"
 *
 *
 *    (U8) HI:D[15..8]     (U9) LO:D[7..0]
 *         ______               ______
 *        |  D14 |             |  D6  |
 *        |      |             |      |
 *      D9|      |D13        D1|      |D5
 *        |______|             |______|                  ___
 *        |  D8  |             |  D0  |                 | A |
 *        |      |             |      |                F|___|B
 *     D10|      |D12        D2|      |D4               | G |
 *        |______|             |______|                E|___|C
 *           D11  *               D3   *                  D  *
 *                D15                  D7                    DP
 *
 */
#define	SEVENSEG_DIGIT_HI_LO_EQUAL	1	/* high nibble equal low nibble */
#define	SEVENSEG_DIGIT_A		(1 << 6) /* bit 6 is segment A */
#define	SEVENSEG_DIGIT_B		(1 << 5) /* bit 5 is segment B */
#define	SEVENSEG_DIGIT_C		(1 << 4) /* bit 4 is segment C */
#define	SEVENSEG_DIGIT_D		(1 << 3) /* bit 3 is segment D */
#define	SEVENSEG_DIGIT_E		(1 << 2) /* bit 2 is segment E */
#define	SEVENSEG_DIGIT_F		(1 << 1) /* bit 1 is segment F */
#define	SEVENSEG_DIGIT_G		(1 << 0) /* bit 0 is segment G */
#define	SEVENSEG_DIGIT_DP		(1 << 7) /* bit 7 is decimal point */

#endif	/* CONFIG_SYS_NIOS_CPU_PIO_NUMS */

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

#define CONFIG_CMD_CDP
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_DISPLAY
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_PING
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_REISER
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_XIMG

/*------------------------------------------------------------------------
 * KGDB
 *----------------------------------------------------------------------*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	9600
#endif

/*------------------------------------------------------------------------
 * MISC
 *----------------------------------------------------------------------*/
#define	CONFIG_SYS_LONGHELP			    /* undef to save memory	*/
#define	CONFIG_SYS_PROMPT		"DK1S10 > " /* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256	    /* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16	    /* max number of command args*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE  /* Boot Argument Buffer Size */

/* Default load address	*/
#if	(CONFIG_SYS_SRAM_SIZE != 0)

/* default in SRAM */
#define	CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SRAM_BASE

#elif	(CONFIG_SYS_SDRAM_SIZE != 0)

/* default in SDRAM */
#if	(CONFIG_SYS_SDRAM_BASE == CONFIG_SYS_NIOS_CPU_VEC_BASE)
#define	CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_NIOS_CPU_VEC_SIZE)
#else
#define	CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE
#endif

#else
#undef	CONFIG_SYS_LOAD_ADDR		/* force error break */
#endif


/* MEM test area */
#if	(CONFIG_SYS_SDRAM_SIZE != 0)

/* SDRAM begin to stack area (1MB stack) */
#if	(CONFIG_SYS_SDRAM_BASE == CONFIG_SYS_NIOS_CPU_VEC_BASE)
#define	CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_NIOS_CPU_VEC_SIZE)
#define	CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP - (1024 * 1024))
#else
#define	CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define	CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP - (1024 * 1024))
#endif

#else
#undef	CONFIG_SYS_MEMTEST_START	/* force error break */
#undef	CONFIG_SYS_MEMTEST_END
#endif

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif	/* __CONFIG_H */
