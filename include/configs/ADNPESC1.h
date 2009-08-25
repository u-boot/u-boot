/*
 * (C) Copyright 2004, Li-Pro.Net <www.li-pro.net>
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

#if	defined(CONFIG_NIOS_BASE_32)
#include <configs/ADNPESC1_base_32.h>
#else
#error *** CONFIG_SYS_ERROR: you have to setup right NIOS CPU configuration
#endif

/*------------------------------------------------------------------------
 * BOARD/CPU -- TOP-LEVEL
 *----------------------------------------------------------------------*/
#define CONFIG_NIOS		1		/* NIOS-32 core		*/
#define	CONFIG_ADNPESC1		1		/* SSV ADNP/ESC1 board	*/
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
 * appropriately -- this is very important if you plan to move your
 * memory to another place as configured at this time !!!).
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
#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned short	/* flash word size	*/

#else
#error *** CONFIG_SYS_ERROR: you have to setup any Flash memory in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * ENVIRONMENT
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_FLASH_SIZE != 0)

#define	CONFIG_ENV_IS_IN_FLASH	1		/* Environment in flash */

/* Mem addr of environment */
#if	defined(CONFIG_NIOS_BASE_32)
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
 * NIOS APPLICATION CODE BASE AREA
 *----------------------------------------------------------------------*/
#if	((CONFIG_ENV_ADDR + CONFIG_ENV_SIZE) == 0x1050000)
#define	CONFIG_SYS_ADNPESC1_UPDATE_LOAD_ADDR	"0x2000100"
#define CONFIG_SYS_ADNPESC1_NIOS_APPL_ENTRY	"0x1050000"
#define CONFIG_SYS_ADNPESC1_NIOS_APPL_IDENT	"0x105000c"
#define	CONFIG_SYS_ADNPESC1_NIOS_APPL_END	"0x11fffff"
#define CONFIG_SYS_ADNPESC1_FILESYSTEM_BASE	"0x1200000"
#define	CONFIG_SYS_ADNPESC1_FILESYSTEM_END	"0x17fffff"
#else
#error *** CONFIG_SYS_ERROR: missing right appl.code base configuration, expand your config.h
#endif
#define CONFIG_SYS_ADNPESC1_NIOS_IDENTIFIER	"Nios"

/*------------------------------------------------------------------------
 * BOOT ENVIRONMENT
 *----------------------------------------------------------------------*/
#ifdef	CONFIG_DNPEVA2			/* DNP/EVA2 base board */
#define	CONFIG_SYS_ADNPESC1_SLED_BOOT_OFF	"sled boot off; "
#define	CONFIG_SYS_ADNPESC1_SLED_RED_BLINK	"sled red blink; "
#else
#define	CONFIG_SYS_ADNPESC1_SLED_BOOT_OFF
#define	CONFIG_SYS_ADNPESC1_SLED_RED_BLINK
#endif

#define	CONFIG_BOOTDELAY	5
#define	CONFIG_BOOTCOMMAND						\
	"if itest.s *$appl_ident_addr == \"$appl_ident_str\"; "		\
	"then "								\
		"wd off; "						\
		CONFIG_SYS_ADNPESC1_SLED_BOOT_OFF				\
		"go $appl_entry_addr; "					\
	"else "								\
		CONFIG_SYS_ADNPESC1_SLED_RED_BLINK				\
		"echo *** missing \"$appl_ident_str\" at $appl_ident_addr; "\
		"echo *** invalid application at $appl_entry_addr; "	\
		"echo *** stop bootup...; "				\
	"fi"

/*------------------------------------------------------------------------
 * EXTRA ENVIRONMENT
 *----------------------------------------------------------------------*/
#ifdef	CONFIG_DNPEVA2			/* DNP/EVA2 base board */
#define	CONFIG_SYS_ADNPESC1_SLED_YELLO_ON	"sled yellow on; "
#define	CONFIG_SYS_ADNPESC1_SLED_YELLO_OFF	"sled yellow off; "
#else
#define	CONFIG_SYS_ADNPESC1_SLED_YELLO_ON
#define	CONFIG_SYS_ADNPESC1_SLED_YELLO_OFF
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"update_allowed=0\0"						\
	"update_load_addr="	CONFIG_SYS_ADNPESC1_UPDATE_LOAD_ADDR	"\0"	\
	"appl_entry_addr="	CONFIG_SYS_ADNPESC1_NIOS_APPL_ENTRY	"\0"	\
	"appl_end_addr="	CONFIG_SYS_ADNPESC1_NIOS_APPL_END	"\0"	\
	"appl_ident_addr="	CONFIG_SYS_ADNPESC1_NIOS_APPL_IDENT	"\0"	\
	"appl_ident_str="	CONFIG_SYS_ADNPESC1_NIOS_IDENTIFIER	"\0"	\
	"appl_name=ADNPESC1/base32/linux.bin\0"				\
	"appl_update="							\
		"if itest.b $update_allowed != 0; "			\
		"then "							\
			CONFIG_SYS_ADNPESC1_SLED_YELLO_ON			\
			"tftp $update_load_addr $appl_name; "		\
			"protect off $appl_entry_addr $appl_end_addr; "	\
			"era $appl_entry_addr $appl_end_addr; "		\
			"cp.b $update_load_addr $appl_entry_addr $filesize; "\
			CONFIG_SYS_ADNPESC1_SLED_YELLO_OFF			\
		"else "							\
			"echo *** update not allowed (update_allowed=$update_allowed); "\
		"fi\0"							\
	"fs_base_addr="		CONFIG_SYS_ADNPESC1_FILESYSTEM_BASE	"\0"	\
	"fs_end_addr="		CONFIG_SYS_ADNPESC1_FILESYSTEM_END	"\0"	\
	"fs_name=ADNPESC1/base32/romfs.img\0"				\
	"fs_update="							\
		"if itest.b $update_allowed != 0; "			\
		"then "							\
			CONFIG_SYS_ADNPESC1_SLED_YELLO_ON			\
			"tftp $update_load_addr $fs_name; "		\
			"protect off $fs_base_addr $fs_end_addr; "	\
			"era $fs_base_addr $fs_end_addr; "		\
			"cp.b $update_load_addr $fs_base_addr $filesize; "\
			CONFIG_SYS_ADNPESC1_SLED_YELLO_OFF			\
		"else "							\
			"echo *** update not allowed (update_allowed=$update_allowed); "\
		"fi\0"							\
	"uboot_name=ADNPESC1/base32/u-boot.bin\0"			\
	"uboot_loadnrun="						\
		"if ping $serverip; "					\
		"then "							\
			CONFIG_SYS_ADNPESC1_SLED_YELLO_ON			\
			"tftp $update_load_addr $uboot_name; "		\
			"wd off; "					\
			"go $update_load_addr; "			\
		"else "							\
			"echo *** missing connection to $serverip; "	\
			"echo *** check your network and try again...; "\
		"fi\0"

/*------------------------------------------------------------------------
 * CONSOLE
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_UART_NUMS != 0)

#define CONFIG_SYS_NIOS_CONSOLE	CONFIG_SYS_NIOS_CPU_UART0 /* 1st UART is Cons. */

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
 * WATCHDOG (or better MAX823 supervisory circuite access)
 *----------------------------------------------------------------------*/
#define	CONFIG_HW_WATCHDOG	1		/* board specific WD	*/

#ifdef	CONFIG_HW_WATCHDOG

/* MAX823 supervisor -- watchdog enable port at: */
#if	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 0)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO0	/* PIO0		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 1)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO1	/* PIO1		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 2)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO2	/* PIO2		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 3)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO3	/* PIO3		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 4)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO4	/* PIO4		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 5)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO5	/* PIO5		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 6)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO6	/* PIO6		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 7)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO7	/* PIO7		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 8)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO8	/* PIO8		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDENA_PIO == 9)
#define	CONFIG_HW_WDENA_BASE	CONFIG_SYS_NIOS_CPU_PIO9	/* PIO9		*/
#else
#error *** CONFIG_SYS_ERROR: you have to setup at least one WDENA_PIO in NIOS CPU config
#endif

/* MAX823 supervisor -- watchdog trigger port at: */
#if	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 0)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO0	/* PIO0		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 1)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO1	/* PIO1		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 2)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO2	/* PIO2		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 3)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO3	/* PIO3		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 4)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO4	/* PIO4		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 5)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO5	/* PIO5		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 6)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO6	/* PIO6		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 7)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO7	/* PIO7		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 8)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO8	/* PIO8		*/
#elif	(CONFIG_SYS_NIOS_CPU_WDTOG_PIO == 9)
#define	CONFIG_HW_WDTOG_BASE	CONFIG_SYS_NIOS_CPU_PIO9	/* PIO9		*/
#else
#error *** CONFIG_SYS_ERROR: you have to setup at least one WDTOG_PIO in NIOS CPU config
#endif

#if	defined(CONFIG_NIOS_BASE_32)		/* NIOS CPU specifics	*/
#define	CONFIG_HW_WDENA_BIT		0	/* WD enable  @ Bit 0	*/
#define	CONFIG_HW_WDTOG_BIT		0	/* WD trigger @ Bit 0	*/
#define	CONFIG_HW_WDPORT_WRONLY	1	/* each WD port wr/only*/
#else
#error *** CONFIG_SYS_ERROR: missing watchdog bit configuration, expand your config.h
#endif

#endif	/* CONFIG_HW_WATCHDOG */

/*------------------------------------------------------------------------
 * SERIAL PERIPHAREL INTERFACE
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_SPI_NUMS == 1)

#define	CONFIG_NIOS_SPI		1		/* SPI support active	*/
#define	CONFIG_SYS_NIOS_SPIBASE	CONFIG_SYS_NIOS_CPU_SPI0
#define	CONFIG_SYS_NIOS_SPIBITS	CONFIG_SYS_NIOS_CPU_SPI0_BITS

#define	CONFIG_RTC_DS1306	1	/* Dallas 1306 real time clock	*/
#define CONFIG_SYS_SPI_RTC_DEVID	0	/*        as 1st SPI device	*/

#else
#undef	CONFIG_NIOS_SPI				/* NO SPI support	*/
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
#define CONFIG_NET_MULTI
#define	CONFIG_CS8900		/* Using CS8900		*/
#define	CONFIG_CS8900_BASE	(CONFIG_SYS_NIOS_CPU_LAN0_BASE + \
				CONFIG_SYS_NIOS_CPU_LAN0_OFFS)

#if	(CONFIG_SYS_NIOS_CPU_LAN0_BUSW == 32)
#undef	CONFIG_CS8900_BUS16
#define	CONFIG_CS8900_BUS32
#else	/* no */
#define	CONFIG_CS8900_BUS16
#undef	CONFIG_CS8900_BUS32
#endif

#else
#error *** CONFIG_SYS_ERROR: invalid LAN0 chip type, check your NIOS CPU config
#endif

#define CONFIG_ETHADDR		02:80:ae:20:60:6f
#define CONFIG_NETMASK		255.255.255.248
#define CONFIG_IPADDR		192.168.161.84
#define CONFIG_SERVERIP		192.168.161.85

#else
#error *** CONFIG_SYS_ERROR: you have to setup just one LAN only or expand your config.h
#endif

/*------------------------------------------------------------------------
 * STATUS LEDs
 *----------------------------------------------------------------------*/
#if	(CONFIG_SYS_NIOS_CPU_PIO_NUMS != 0) && defined(CONFIG_SYS_NIOS_CPU_LED_PIO)

#if	(CONFIG_SYS_NIOS_CPU_LED_PIO == 0)

#define	STATUS_LED_BASE			CONFIG_SYS_NIOS_CPU_PIO0
#define	STATUS_LED_BITS			CONFIG_SYS_NIOS_CPU_PIO0_BITS
#define	STATUS_LED_ACTIVE		1 /* LED on for bit == 1 */

#if	(CONFIG_SYS_NIOS_CPU_PIO0_TYPE == 1)
#define	STATUS_LED_WRONLY		1
#else
#undef	STATUS_LED_WRONLY
#endif

#elif	(CONFIG_SYS_NIOS_CPU_LED_PIO == 1)

#define	STATUS_LED_BASE			CONFIG_SYS_NIOS_CPU_PIO1
#define	STATUS_LED_BITS			CONFIG_SYS_NIOS_CPU_PIO1_BITS
#define	STATUS_LED_ACTIVE		1 /* LED on for bit == 1 */

#if	(CONFIG_SYS_NIOS_CPU_PIO1_TYPE == 1)
#define	STATUS_LED_WRONLY		1
#else
#undef	STATUS_LED_WRONLY
#endif

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
#define	STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)	/* ca. 1 Hz */
#define	STATUS_LED_BOOT			0		/* boot LED */

#if	(STATUS_LED_BITS > 1)
#define	STATUS_LED_BIT1			(1 << 1)	/* LED[1] */
#define	STATUS_LED_STATE1		STATUS_LED_OFF
#define	STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 10)	/* ca. 5 Hz */
#define	STATUS_LED_RED			1		/* fail LED */
#endif

#if	(STATUS_LED_BITS > 2)
#define	STATUS_LED_BIT2			(1 << 2)	/* LED[2] */
#define	STATUS_LED_STATE2		STATUS_LED_OFF
#define	STATUS_LED_PERIOD2		(CONFIG_SYS_HZ / 2)	/* ca. 1 Hz */
#define	STATUS_LED_YELLOW		2		/* info LED */
#endif

#if	(STATUS_LED_BITS > 3)
#define	STATUS_LED_BIT3			(1 << 3)	/* LED[3] */
#define	STATUS_LED_STATE3		STATUS_LED_OFF
#define	STATUS_LED_PERIOD3		(CONFIG_SYS_HZ / 2)	/* ca. 1 Hz */
#define	STATUS_LED_GREEN		3		/* info LED */
#endif

#define	STATUS_LED_PAR			1 /* makes status_led.h happy */

#endif	/* CONFIG_SYS_NIOS_CPU_PIO_NUMS */

/*------------------------------------------------------------------------
 * Diagnostics / Power On Self Tests
 *----------------------------------------------------------------------*/
#define	CONFIG_POST			CONFIG_SYS_POST_RTC
#define	CONFIG_SYS_NIOS_POST_WORD_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

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

#define CONFIG_CMD_BSP
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

#if (CONFIG_SYS_NIOS_CPU_SPI_NUMS == 1)
#define CONFIG_CMD_DATE
#define CONFIG_CMD_SPI
#endif

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
#define	CONFIG_SYS_HUSH_PARSER		1	    /* use "hush" command parser
					       undef to save memory	*/
#define	CONFIG_SYS_PROMPT		"ADNPESC1 > " /* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		1024	    /* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		64	    /* max number of command args*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE  /* Boot Argument Buffer Size */

#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"[]> "
#endif

/* Default load address	*/
#if	(CONFIG_SYS_SRAM_SIZE != 0)

/* default in SRAM */
#define	CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SRAM_BASE

#elif	(CONFIG_SYS_SDRAM_SIZE != 0)

/* default in SDRAM */
#if	(CONFIG_SYS_SDRAM_BASE == CONFIG_SYS_NIOS_CPU_VEC_BASE)
#if 1
#define	CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_NIOS_CPU_VEC_SIZE)
#else
#define	CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x400000)
#endif
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
#if 0
#define	CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_NIOS_CPU_VEC_SIZE)
#else
#define	CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x400000)
#endif
#else
#define	CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#endif

#define	CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP - (1024 * 1024))
#define	CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP - (1024 * 1024))

#else
#undef	CONFIG_SYS_MEMTEST_START	/* force error break */
#undef	CONFIG_SYS_MEMTEST_END
#endif

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif	/* __CONFIG_H */
