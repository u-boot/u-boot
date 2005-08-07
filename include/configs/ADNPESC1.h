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
#error *** CFG_ERROR: you have to setup right NIOS CPU configuration
#endif

/*------------------------------------------------------------------------
 * BOARD/CPU -- TOP-LEVEL
 *----------------------------------------------------------------------*/
#define CONFIG_NIOS		1		/* NIOS-32 core		*/
#define	CONFIG_ADNPESC1		1		/* SSV ADNP/ESC1 board	*/
#define CONFIG_SYS_CLK_FREQ	CFG_NIOS_CPU_CLK/* 50 MHz core clock	*/
#define	CFG_HZ			1000		/* 1 msec time tick	*/
#undef  CFG_CLKS_IN_HZ
#define	CONFIG_BOARD_EARLY_INIT_F 1	/* enable early board-spec. init*/

/*------------------------------------------------------------------------
 * BASE ADDRESSES / SIZE (Flash, SRAM, SDRAM)
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_SDRAM_SIZE != 0)

#define CFG_SDRAM_BASE		CFG_NIOS_CPU_SDRAM_BASE
#define CFG_SDRAM_SIZE		CFG_NIOS_CPU_SDRAM_SIZE

#else
#error *** CFG_ERROR: you have to setup any SDRAM in NIOS CPU config
#endif

#if	defined(CFG_NIOS_CPU_SRAM_BASE) && defined(CFG_NIOS_CPU_SRAM_SIZE)

#define	CFG_SRAM_BASE		CFG_NIOS_CPU_SRAM_BASE
#define	CFG_SRAM_SIZE		CFG_NIOS_CPU_SRAM_SIZE

#else

#undef	CFG_SRAM_BASE
#undef	CFG_SRAM_SIZE

#endif

#define CFG_VECT_BASE		CFG_NIOS_CPU_VEC_BASE

/*------------------------------------------------------------------------
 * MEMORY ORGANIZATION - For the most part, you can put things pretty
 * much anywhere. This is pretty flexible for Nios. So here we make some
 * arbitrary choices & assume that the monitor is placed at the end of
 * a memory resource (so you must make sure TEXT_BASE is chosen
 * appropriately -- this is very important if you plan to move your
 * memory to another place as configured at this time !!!).
 *
 * 	-The heap is placed below the monitor.
 * 	-Global data is placed below the heap.
 * 	-The stack is placed below global data (&grows down).
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256k		*/
#define CFG_GBL_DATA_SIZE	128		/* Global data size rsvd*/
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)
#define CFG_GBL_DATA_OFFSET	(CFG_MALLOC_BASE - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP		CFG_GBL_DATA_OFFSET

/*------------------------------------------------------------------------
 * FLASH (AM29LV065D)
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_FLASH_SIZE != 0)

#define CFG_FLASH_BASE		CFG_NIOS_CPU_FLASH_BASE
#define CFG_FLASH_SIZE		CFG_NIOS_CPU_FLASH_SIZE
#define CFG_MAX_FLASH_SECT	128		/* Max # sects per bank */
#define CFG_MAX_FLASH_BANKS	1		/* Max # of flash banks */
#define CFG_FLASH_ERASE_TOUT	8000		/* Erase timeout (msec) */
#define CFG_FLASH_WRITE_TOUT	100		/* Write timeout (msec) */
#define CFG_FLASH_WORD_SIZE	unsigned short	/* flash word size	*/

#else
#error *** CFG_ERROR: you have to setup any Flash memory in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * ENVIRONMENT
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_FLASH_SIZE != 0)

#define	CFG_ENV_IS_IN_FLASH	1		/* Environment in flash */

/* Mem addr of environment */
#if	defined(CONFIG_NIOS_BASE_32)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + CFG_MONITOR_LEN)
#else
#error *** CFG_ERROR: you have to setup the environment base address CFG_ENV_ADDR
#endif

#define CFG_ENV_SIZE		(64 * 1024)	/* 64 KByte (1 sector)	*/
#define CONFIG_ENV_OVERWRITE			/* Serial/eth change Ok */

#else
#define	CFG_ENV_IS_NOWHERE	1		/* NO Environment	*/
#endif

/*------------------------------------------------------------------------
 * NIOS APPLICATION CODE BASE AREA
 *----------------------------------------------------------------------*/
#if	((CFG_ENV_ADDR + CFG_ENV_SIZE) == 0x1050000)
#define	CFG_ADNPESC1_UPDATE_LOAD_ADDR	"0x2000100"
#define CFG_ADNPESC1_NIOS_APPL_ENTRY	"0x1050000"
#define CFG_ADNPESC1_NIOS_APPL_IDENT	"0x105000c"
#define	CFG_ADNPESC1_NIOS_APPL_END	"0x11fffff"
#define CFG_ADNPESC1_FILESYSTEM_BASE	"0x1200000"
#define	CFG_ADNPESC1_FILESYSTEM_END	"0x17fffff"
#else
#error *** CFG_ERROR: missing right appl.code base configuration, expand your config.h
#endif
#define CFG_ADNPESC1_NIOS_IDENTIFIER	"Nios"

/*------------------------------------------------------------------------
 * BOOT ENVIRONMENT
 *----------------------------------------------------------------------*/
#ifdef	CONFIG_DNPEVA2			/* DNP/EVA2 base board */
#define	CFG_ADNPESC1_SLED_BOOT_OFF	"sled boot off; "
#define	CFG_ADNPESC1_SLED_RED_BLINK	"sled red blink; "
#else
#define	CFG_ADNPESC1_SLED_BOOT_OFF
#define	CFG_ADNPESC1_SLED_RED_BLINK
#endif

#define	CONFIG_BOOTDELAY	5
#define	CONFIG_BOOTCOMMAND						\
	"if itest.s *$appl_ident_addr == \"$appl_ident_str\"; "		\
	"then "								\
		"wd off; "						\
		CFG_ADNPESC1_SLED_BOOT_OFF				\
		"go $appl_entry_addr; "					\
	"else "								\
		CFG_ADNPESC1_SLED_RED_BLINK				\
		"echo *** missing \"$appl_ident_str\" at $appl_ident_addr; "\
		"echo *** invalid application at $appl_entry_addr; "	\
		"echo *** stop bootup...; "				\
	"fi"

/*------------------------------------------------------------------------
 * EXTRA ENVIRONMENT
 *----------------------------------------------------------------------*/
#ifdef	CONFIG_DNPEVA2			/* DNP/EVA2 base board */
#define	CFG_ADNPESC1_SLED_YELLO_ON	"sled yellow on; "
#define	CFG_ADNPESC1_SLED_YELLO_OFF	"sled yellow off; "
#else
#define	CFG_ADNPESC1_SLED_YELLO_ON
#define	CFG_ADNPESC1_SLED_YELLO_OFF
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"update_allowed=0\0"						\
	"update_load_addr="	CFG_ADNPESC1_UPDATE_LOAD_ADDR	"\0"	\
	"appl_entry_addr="	CFG_ADNPESC1_NIOS_APPL_ENTRY	"\0"	\
	"appl_end_addr="	CFG_ADNPESC1_NIOS_APPL_END	"\0"	\
	"appl_ident_addr="	CFG_ADNPESC1_NIOS_APPL_IDENT	"\0"	\
	"appl_ident_str="	CFG_ADNPESC1_NIOS_IDENTIFIER	"\0"	\
	"appl_name=ADNPESC1/base32/linux.bin\0"				\
	"appl_update="							\
		"if itest.b $update_allowed != 0; "			\
		"then "							\
			CFG_ADNPESC1_SLED_YELLO_ON			\
			"tftp $update_load_addr $appl_name; "		\
			"protect off $appl_entry_addr $appl_end_addr; "	\
			"era $appl_entry_addr $appl_end_addr; "		\
			"cp.b $update_load_addr $appl_entry_addr $filesize; "\
			CFG_ADNPESC1_SLED_YELLO_OFF			\
		"else "							\
			"echo *** update not allowed (update_allowed=$update_allowed); "\
		"fi\0"							\
	"fs_base_addr="		CFG_ADNPESC1_FILESYSTEM_BASE	"\0"	\
	"fs_end_addr="		CFG_ADNPESC1_FILESYSTEM_END	"\0"	\
	"fs_name=ADNPESC1/base32/romfs.img\0"				\
	"fs_update="							\
		"if itest.b $update_allowed != 0; "			\
		"then "							\
			CFG_ADNPESC1_SLED_YELLO_ON			\
			"tftp $update_load_addr $fs_name; "		\
			"protect off $fs_base_addr $fs_end_addr; "	\
			"era $fs_base_addr $fs_end_addr; "		\
			"cp.b $update_load_addr $fs_base_addr $filesize; "\
			CFG_ADNPESC1_SLED_YELLO_OFF			\
		"else "							\
			"echo *** update not allowed (update_allowed=$update_allowed); "\
		"fi\0"							\
	"uboot_name=ADNPESC1/base32/u-boot.bin\0"			\
	"uboot_loadnrun="						\
		"if ping $serverip; "					\
		"then "							\
			CFG_ADNPESC1_SLED_YELLO_ON			\
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
#if	(CFG_NIOS_CPU_UART_NUMS != 0)

#define CFG_NIOS_CONSOLE	CFG_NIOS_CPU_UART0 /* 1st UART is Cons. */

#if	(CFG_NIOS_CPU_UART0_BR != 0)
#define CFG_NIOS_FIXEDBAUD	1		   /* Baudrate is fixed	*/
#define CONFIG_BAUDRATE		CFG_NIOS_CPU_UART0_BR
#else
#undef	CFG_NIOS_FIXEDBAUD
#define CONFIG_BAUDRATE		115200
#endif

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#else
#error *** CFG_ERROR: you have to setup at least one UART in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * TIMER FOR TIMEBASE -- Nios doesn't have the equivalent of ppc  PIT,
 * so an avalon bus timer is required.
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_TIMER_NUMS != 0) && defined(CFG_NIOS_CPU_TICK_TIMER)

#if	(CFG_NIOS_CPU_TICK_TIMER == 0)

#define CFG_NIOS_TMRBASE	CFG_NIOS_CPU_TIMER0 /* TIMER0 as tick	*/
#define CFG_NIOS_TMRIRQ		CFG_NIOS_CPU_TIMER0_IRQ

#if	(CFG_NIOS_CPU_TIMER0_FP == 1)		    /* fixed period */

#if	(CFG_NIOS_CPU_TIMER0_PER >= CFG_HZ)
#define CFG_NIOS_TMRMS		(CFG_NIOS_CPU_TIMER0_PER / CFG_HZ)
#else
#error *** CFG_ERROR: you have to use a timer periode greater than CFG_HZ
#endif

#undef	CFG_NIOS_TMRCNT	/* no preloadable counter value */

#elif	(CFG_NIOS_CPU_TIMER0_FP == 0)		    /* variable period */

#if	(CFG_HZ <= 1000)
#define CFG_NIOS_TMRMS		(1000 / CFG_HZ)
#else
#error *** CFG_ERROR: sorry, CFG_HZ have to be less than 1000
#endif

#define	CFG_NIOS_TMRCNT		(CONFIG_SYS_CLK_FREQ / CFG_HZ)

#else
#error *** CFG_ERROR: you have to define CFG_NIOS_CPU_TIMER0_FP correct
#endif

#elif	(CFG_NIOS_CPU_TICK_TIMER == 1)

#define CFG_NIOS_TMRBASE	CFG_NIOS_CPU_TIMER1 /* TIMER1 as tick	*/
#define CFG_NIOS_TMRIRQ		CFG_NIOS_CPU_TIMER1_IRQ

#if	(CFG_NIOS_CPU_TIMER1_FP == 1)		    /* fixed period */

#if	(CFG_NIOS_CPU_TIMER1_PER >= CFG_HZ)
#define CFG_NIOS_TMRMS		(CFG_NIOS_CPU_TIMER1_PER / CFG_HZ)
#else
#error *** CFG_ERROR: you have to use a timer periode greater than CFG_HZ
#endif

#undef	CFG_NIOS_TMRCNT	/* no preloadable counter value */

#elif	(CFG_NIOS_CPU_TIMER1_FP == 0)		    /* variable period */

#if	(CFG_HZ <= 1000)
#define CFG_NIOS_TMRMS		(1000 / CFG_HZ)
#else
#error *** CFG_ERROR: sorry, CFG_HZ have to be less than 1000
#endif

#define	CFG_NIOS_TMRCNT		(CONFIG_SYS_CLK_FREQ / CFG_HZ)

#else
#error *** CFG_ERROR: you have to define CFG_NIOS_CPU_TIMER1_FP correct
#endif

#endif	/* CFG_NIOS_CPU_TICK_TIMER */

#else
#error *** CFG_ERROR: you have to setup at least one TIMER in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * WATCHDOG (or better MAX823 supervisory circuite access)
 *----------------------------------------------------------------------*/
#define	CONFIG_HW_WATCHDOG	1		/* board specific WD	*/

#ifdef	CONFIG_HW_WATCHDOG

/* MAX823 supervisor -- watchdog enable port at: */
#if	(CFG_NIOS_CPU_WDENA_PIO == 0)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO0	/* PIO0		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 1)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO1	/* PIO1		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 2)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO2	/* PIO2		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 3)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO3	/* PIO3		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 4)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO4	/* PIO4		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 5)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO5	/* PIO5		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 6)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO6	/* PIO6		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 7)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO7	/* PIO7		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 8)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO8	/* PIO8		*/
#elif	(CFG_NIOS_CPU_WDENA_PIO == 9)
#define	CONFIG_HW_WDENA_BASE	CFG_NIOS_CPU_PIO9	/* PIO9		*/
#else
#error *** CFG_ERROR: you have to setup at least one WDENA_PIO in NIOS CPU config
#endif

/* MAX823 supervisor -- watchdog trigger port at: */
#if	(CFG_NIOS_CPU_WDTOG_PIO == 0)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO0	/* PIO0		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 1)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO1	/* PIO1		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 2)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO2	/* PIO2		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 3)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO3	/* PIO3		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 4)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO4	/* PIO4		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 5)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO5	/* PIO5		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 6)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO6	/* PIO6		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 7)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO7	/* PIO7		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 8)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO8	/* PIO8		*/
#elif	(CFG_NIOS_CPU_WDTOG_PIO == 9)
#define	CONFIG_HW_WDTOG_BASE	CFG_NIOS_CPU_PIO9	/* PIO9		*/
#else
#error *** CFG_ERROR: you have to setup at least one WDTOG_PIO in NIOS CPU config
#endif

#if	defined(CONFIG_NIOS_BASE_32)		/* NIOS CPU specifics	*/
#define	CONFIG_HW_WDENA_BIT		0	/* WD enable  @ Bit 0	*/
#define	CONFIG_HW_WDTOG_BIT		0	/* WD trigger @ Bit 0	*/
#define	CONFIG_HW_WDPORT_WRONLY	1	/* each WD port wr/only*/
#else
#error *** CFG_ERROR: missing watchdog bit configuration, expand your config.h
#endif

#endif	/* CONFIG_HW_WATCHDOG */

/*------------------------------------------------------------------------
 * SERIAL PERIPHAREL INTERFACE
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_SPI_NUMS == 1)

#define	CONFIG_NIOS_SPI		1		/* SPI support active	*/
#define	CFG_NIOS_SPIBASE	CFG_NIOS_CPU_SPI0
#define	CFG_NIOS_SPIBITS	CFG_NIOS_CPU_SPI0_BITS

#define	CONFIG_RTC_DS1306	1	/* Dallas 1306 real time clock	*/
#define CFG_SPI_RTC_DEVID	0	/*        as 1st SPI device	*/

#define	__SPI_CMD_OFF		0	/* allow default commands:	*/
					/*	CFG_CMD_SPI		*/
					/*	CFG_CMD_DATE		*/

#else
#undef	CONFIG_NIOS_SPI				/* NO SPI support	*/
#define	__SPI_CMD_OFF	(	CFG_CMD_SPI	\
			|	CFG_CMD_DATE	\
			)
#endif

/*------------------------------------------------------------------------
 * Ethernet -- needs work!
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_LAN_NUMS == 1)

#if	(CFG_NIOS_CPU_LAN0_TYPE == 0)		/* LAN91C111		*/

#define	CONFIG_DRIVER_SMC91111			/* Using SMC91c111	*/
#undef	CONFIG_SMC91111_EXT_PHY			/* Internal PHY		*/
#define	CONFIG_SMC91111_BASE	(CFG_NIOS_CPU_LAN0_BASE + CFG_NIOS_CPU_LAN0_OFFS)

#if	(CFG_NIOS_CPU_LAN0_BUSW == 32)
#define	CONFIG_SMC_USE_32_BIT	1
#else	/* no */
#undef	CONFIG_SMC_USE_32_BIT
#endif

#elif	(CFG_NIOS_CPU_LAN0_TYPE == 1)		/* CS8900A		*/

	/********************************************/
	/* !!! CS8900 is __not__ tested on NIOS !!! */
	/********************************************/
#define	CONFIG_DRIVER_CS8900			/* Using CS8900		*/
#define	CS8900_BASE		(CFG_NIOS_CPU_LAN0_BASE + CFG_NIOS_CPU_LAN0_OFFS)

#if	(CFG_NIOS_CPU_LAN0_BUSW == 32)
#undef	CS8900_BUS16
#define	CS8900_BUS32		1
#else	/* no */
#define	CS8900_BUS16		1
#undef	CS8900_BUS32
#endif

#else
#error *** CFG_ERROR: invalid LAN0 chip type, check your NIOS CPU config
#endif

#define CONFIG_ETHADDR		02:80:ae:20:60:6f
#define CONFIG_NETMASK		255.255.255.248
#define CONFIG_IPADDR		192.168.161.84
#define CONFIG_SERVERIP		192.168.161.85

#else
#error *** CFG_ERROR: you have to setup just one LAN only or expand your config.h
#endif

/*------------------------------------------------------------------------
 * STATUS LEDs
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_PIO_NUMS != 0) && defined(CFG_NIOS_CPU_LED_PIO)

#if	(CFG_NIOS_CPU_LED_PIO == 0)

#define	STATUS_LED_BASE			CFG_NIOS_CPU_PIO0
#define	STATUS_LED_BITS			CFG_NIOS_CPU_PIO0_BITS
#define	STATUS_LED_ACTIVE		1 /* LED on for bit == 1 */

#if	(CFG_NIOS_CPU_PIO0_TYPE == 1)
#define	STATUS_LED_WRONLY		1
#else
#undef	STATUS_LED_WRONLY
#endif

#elif	(CFG_NIOS_CPU_LED_PIO == 1)

#define	STATUS_LED_BASE			CFG_NIOS_CPU_PIO1
#define	STATUS_LED_BITS			CFG_NIOS_CPU_PIO1_BITS
#define	STATUS_LED_ACTIVE		1 /* LED on for bit == 1 */

#if	(CFG_NIOS_CPU_PIO1_TYPE == 1)
#define	STATUS_LED_WRONLY		1
#else
#undef	STATUS_LED_WRONLY
#endif

#elif	(CFG_NIOS_CPU_LED_PIO == 2)

#define	STATUS_LED_BASE			CFG_NIOS_CPU_PIO2
#define	STATUS_LED_BITS			CFG_NIOS_CPU_PIO2_BITS
#define	STATUS_LED_ACTIVE		1 /* LED on for bit == 1 */

#if	(CFG_NIOS_CPU_PIO2_TYPE == 1)
#define	STATUS_LED_WRONLY		1
#else
#undef	STATUS_LED_WRONLY
#endif

#elif	(CFG_NIOS_CPU_LED_PIO == 3)

#error *** CFG_ERROR: status LEDs at PIO3 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 4)

#error *** CFG_ERROR: status LEDs at PIO4 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 5)

#error *** CFG_ERROR: status LEDs at PIO5 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 6)

#error *** CFG_ERROR: status LEDs at PIO6 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 7)

#error *** CFG_ERROR: status LEDs at PIO7 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 8)

#error *** CFG_ERROR: status LEDs at PIO8 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 9)

#error *** CFG_ERROR: status LEDs at PIO9 not supported, expand your config.h

#else
#error *** CFG_ERROR: you have to set CFG_NIOS_CPU_LED_PIO in right case
#endif

#define	CONFIG_STATUS_LED		1 /* enable status led driver */

#define	STATUS_LED_BIT			(1 << 0)	/* LED[0] */
#define	STATUS_LED_STATE		STATUS_LED_BLINKING
#define	STATUS_LED_BOOT_STATE		STATUS_LED_OFF
#define	STATUS_LED_PERIOD		(CFG_HZ / 2)	/* ca. 1 Hz */
#define	STATUS_LED_BOOT			0		/* boot LED */

#if	(STATUS_LED_BITS > 1)
#define	STATUS_LED_BIT1			(1 << 1)	/* LED[1] */
#define	STATUS_LED_STATE1		STATUS_LED_OFF
#define	STATUS_LED_PERIOD1		(CFG_HZ / 10)	/* ca. 5 Hz */
#define	STATUS_LED_RED			1		/* fail LED */
#endif

#if	(STATUS_LED_BITS > 2)
#define	STATUS_LED_BIT2			(1 << 2)	/* LED[2] */
#define	STATUS_LED_STATE2		STATUS_LED_OFF
#define	STATUS_LED_PERIOD2		(CFG_HZ / 2)	/* ca. 1 Hz */
#define	STATUS_LED_YELLOW		2		/* info LED */
#endif

#if	(STATUS_LED_BITS > 3)
#define	STATUS_LED_BIT3			(1 << 3)	/* LED[3] */
#define	STATUS_LED_STATE3		STATUS_LED_OFF
#define	STATUS_LED_PERIOD3		(CFG_HZ / 2)	/* ca. 1 Hz */
#define	STATUS_LED_GREEN		3		/* info LED */
#endif

#define	STATUS_LED_PAR			1 /* makes status_led.h happy */

#endif	/* CFG_NIOS_CPU_PIO_NUMS */

/*------------------------------------------------------------------------
 * Diagnostics / Power On Self Tests
 *----------------------------------------------------------------------*/
#define	CONFIG_POST			CFG_POST_RTC
#define	CFG_NIOS_POST_WORD_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)

/*------------------------------------------------------------------------
 * COMMANDS
 *----------------------------------------------------------------------*/
#define CONFIG_COMMANDS		(CFG_CMD_ALL & ~( \
				 CFG_CMD_ASKENV | \
				 CFG_CMD_BEDBUG | \
				 CFG_CMD_BMP	| \
				 CFG_CMD_CACHE	| \
				 CFG_CMD_DOC	| \
				 CFG_CMD_DTT	| \
				 CFG_CMD_EEPROM | \
				 CFG_CMD_ELF    | \
				 CFG_CMD_FAT	| \
				 CFG_CMD_FDC	| \
				 CFG_CMD_FDOS	| \
				 CFG_CMD_HWFLOW	| \
				 CFG_CMD_IDE	| \
				 CFG_CMD_I2C	| \
				 CFG_CMD_JFFS2	| \
				 CFG_CMD_KGDB	| \
				 CFG_CMD_NAND	| \
				 CFG_CMD_NFS	| \
				 CFG_CMD_MMC	| \
				 CFG_CMD_MII	| \
				 CFG_CMD_PCI	| \
				 CFG_CMD_PCMCIA | \
				 CFG_CMD_SCSI	| \
				 CFG_CMD_VFD	| \
				 CFG_CMD_USB	| \
				 CFG_CMD_XIMG	| \
				 __SPI_CMD_OFF	) )


#include <cmd_confdefs.h>

/*------------------------------------------------------------------------
 * KGDB
 *----------------------------------------------------------------------*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	9600
#endif

/*------------------------------------------------------------------------
 * MISC
 *----------------------------------------------------------------------*/
#define	CFG_LONGHELP			    /* undef to save memory	*/
#define	CFG_HUSH_PARSER		1	    /* use "hush" command parser
					       undef to save memory	*/
#define	CFG_PROMPT		"ADNPESC1 > " /* Monitor Command Prompt	*/
#define	CFG_CBSIZE		1024	    /* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		64	    /* max number of command args*/
#define CFG_BARGSIZE		CFG_CBSIZE  /* Boot Argument Buffer Size */

#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"[]> "
#endif

/* Default load address	*/
#if	(CFG_SRAM_SIZE != 0)

/* default in SRAM */
#define	CFG_LOAD_ADDR		CFG_SRAM_BASE

#elif	(CFG_SDRAM_SIZE != 0)

/* default in SDRAM */
#if	(CFG_SDRAM_BASE == CFG_NIOS_CPU_VEC_BASE)
#if 1
#define	CFG_LOAD_ADDR		(CFG_SDRAM_BASE + CFG_NIOS_CPU_VEC_SIZE)
#else
#define	CFG_LOAD_ADDR		(CFG_SDRAM_BASE + 0x400000)
#endif
#else
#define	CFG_LOAD_ADDR		CFG_SDRAM_BASE
#endif

#else
#undef	CFG_LOAD_ADDR		/* force error break */
#endif


/* MEM test area */
#if	(CFG_SDRAM_SIZE != 0)

/* SDRAM begin to stack area (1MB stack) */
#if	(CFG_SDRAM_BASE == CFG_NIOS_CPU_VEC_BASE)
#if 0
#define	CFG_MEMTEST_START	(CFG_SDRAM_BASE + CFG_NIOS_CPU_VEC_SIZE)
#else
#define	CFG_MEMTEST_START	(CFG_SDRAM_BASE + 0x400000)
#endif
#else
#define	CFG_MEMTEST_START	CFG_SDRAM_BASE
#endif

#define	CFG_MEMTEST_END		(CFG_INIT_SP - (1024 * 1024))
#define	CFG_MEMTEST_END		(CFG_INIT_SP - (1024 * 1024))

#else
#undef	CFG_MEMTEST_START	/* force error break */
#undef	CFG_MEMTEST_END
#endif

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif	/* __CONFIG_H */
