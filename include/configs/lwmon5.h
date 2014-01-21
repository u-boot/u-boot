/*
 * (C) Copyright 2007-2013
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * lwmon5.h - configuration for lwmon5 board
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Liebherr extra version info
 */
#define CONFIG_IDENT_STRING	" - v2.0"

/*
 * High Level Configuration Options
 */
#define CONFIG_LWMON5		1		/* Board is lwmon5	*/
#define CONFIG_440EPX		1		/* Specific PPC440EPx	*/
#define CONFIG_440		1		/* ... PPC440 family	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/

#ifdef CONFIG_LCD4_LWMON5
#define	CONFIG_SYS_TEXT_BASE	0x01000000 /* SPL U-Boot TEXT_BASE */
#define CONFIG_HOSTNAME		lcd4_lwmon5
#else
#define CONFIG_SYS_TEXT_BASE	0xFFF80000
#define CONFIG_HOSTNAME		lwmon5
#endif

#define CONFIG_SYS_CLK_FREQ	33300000	/* external freq to pll	*/

#define CONFIG_4xx_DCACHE		/* enable cache in SDRAM	*/

#define CONFIG_BOARD_EARLY_INIT_F	/* Call board_early_init_f	*/
#define CONFIG_BOARD_EARLY_INIT_R	/* Call board_early_init_r	*/
#define CONFIG_BOARD_POSTCLK_INIT	/* Call board_postclk_init	*/
#define CONFIG_MISC_INIT_R		/* Call misc_init_r		*/
#define CONFIG_BOARD_RESET		/* Call board_reset		*/

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* Start of U-Boot	*/
#define CONFIG_SYS_MONITOR_LEN		0x80000
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)	/* Reserved for malloc	*/

#define CONFIG_SYS_BOOT_BASE_ADDR	0xf0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CONFIG_SYS_FLASH_BASE		0xf8000000	/* start of FLASH	*/
#define CONFIG_SYS_LIME_BASE_0		0xc0000000
#define CONFIG_SYS_LIME_BASE_1		0xc1000000
#define CONFIG_SYS_LIME_BASE_2		0xc2000000
#define CONFIG_SYS_LIME_BASE_3		0xc3000000
#define CONFIG_SYS_FPGA_BASE_0		0xc4000000
#define CONFIG_SYS_FPGA_BASE_1		0xc4200000
#define CONFIG_SYS_OCM_BASE		0xe0010000      /* ocm			*/
#define CONFIG_SYS_PCI_BASE		0xe0000000      /* Internal PCI regs	*/
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CONFIG_SYS_PCI_MEMBASE1		(CONFIG_SYS_PCI_MEMBASE  + 0x10000000)
#define CONFIG_SYS_PCI_MEMBASE2		(CONFIG_SYS_PCI_MEMBASE1 + 0x10000000)
#define CONFIG_SYS_PCI_MEMBASE3		(CONFIG_SYS_PCI_MEMBASE2 + 0x10000000)

#ifndef CONFIG_LCD4_LWMON5
#define CONFIG_SYS_USB2D0_BASE		0xe0000100
#define CONFIG_SYS_USB_DEVICE		0xe0000000
#define CONFIG_SYS_USB_HOST		0xe0000400
#endif

/*
 * Initial RAM & stack pointer
 *
 * On LWMON5 we use D-cache as init-ram and stack pointer. We also move
 * the POST_WORD from OCM to a 440EPx register that preserves it's
 * content during reset (GPT0_COMP6). This way we reserve the OCM (16k)
 * for logbuffer only. (GPT0_COMP1-COMP5 are reserved for logbuffer header.)
 */
#ifndef CONFIG_LCD4_LWMON5
#define CONFIG_SYS_INIT_RAM_DCACHE	1		/* d-cache as init ram	*/
#define CONFIG_SYS_INIT_RAM_ADDR	0x70000000		/* DCache       */
#define CONFIG_SYS_INIT_RAM_SIZE		(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#else
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)
#endif
/* unused GPT0 COMP reg	*/
#define CONFIG_SYS_POST_WORD_ADDR	(CONFIG_SYS_PERIPHERAL_BASE + GPT0_COMP6)
#define CONFIG_SYS_OCM_SIZE		(16 << 10)
/* 440EPx errata CHIP 11: don't use last 4kbytes */
#define CONFIG_SYS_MEM_TOP_HIDE		(4 << 10)

/* Additional registers for watchdog timer post test */
#define CONFIG_SYS_WATCHDOG_TIME_ADDR	(CONFIG_SYS_PERIPHERAL_BASE + GPT0_MASK2)
#define CONFIG_SYS_WATCHDOG_FLAGS_ADDR	(CONFIG_SYS_PERIPHERAL_BASE + GPT0_MASK1)
#define CONFIG_SYS_DSPIC_TEST_ADDR	CONFIG_SYS_WATCHDOG_FLAGS_ADDR
#define CONFIG_SYS_OCM_STATUS_ADDR	CONFIG_SYS_WATCHDOG_FLAGS_ADDR
#define CONFIG_SYS_WATCHDOG_MAGIC	0x12480000
#define CONFIG_SYS_WATCHDOG_MAGIC_MASK	0xFFFF0000
#define CONFIG_SYS_DSPIC_TEST_MASK	0x00000001
#define CONFIG_SYS_OCM_STATUS_OK	0x00009A00
#define CONFIG_SYS_OCM_STATUS_FAIL	0x0000A300
#define CONFIG_SYS_OCM_STATUS_MASK	0x0000FF00

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	2	/* Use UART1			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#undef CONFIG_SYS_EXT_SERIAL_CLOCK		/* no external clock provided	*/
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH		/* use FLASH for environment vars	*/

/*
 * FLASH related
 */
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/

#define CONFIG_SYS_FLASH0		0xFC000000
#define CONFIG_SYS_FLASH1		0xF8000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH1, CONFIG_SYS_FLASH0 }

#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT 2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 	/* use buffered writes (20x faster)	*/
#define CONFIG_SYS_FLASH_PROTECTION		/* use hardware flash protection	*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_QUIET_TEST		/* don't warn upon unknown flash	*/

#define CONFIG_ENV_SECT_SIZE	0x40000	/* size of one complete sector		*/
#define CONFIG_ENV_ADDR		((-CONFIG_SYS_MONITOR_LEN) - CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*
 * DDR SDRAM
 */
#define CONFIG_SYS_MBYTES_SDRAM		256
#define CONFIG_SYS_DDR_CACHED_ADDR	0x40000000	/* setup 2nd TLB cached here	*/
#define CONFIG_DDR_DATA_EYE			/* use DDR2 optimization	*/
#ifndef CONFIG_LCD4_LWMON5
#define CONFIG_DDR_ECC				/* enable ECC			*/
#endif

#ifndef CONFIG_LCD4_LWMON5
/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_CACHE		| \
				 CONFIG_SYS_POST_CPU		| \
				 CONFIG_SYS_POST_ECC		| \
				 CONFIG_SYS_POST_ETHER		| \
				 CONFIG_SYS_POST_FPU		| \
				 CONFIG_SYS_POST_I2C		| \
				 CONFIG_SYS_POST_MEMORY		| \
				 CONFIG_SYS_POST_OCM		| \
				 CONFIG_SYS_POST_RTC		| \
				 CONFIG_SYS_POST_SPR		| \
				 CONFIG_SYS_POST_UART		| \
				 CONFIG_SYS_POST_SYSMON		| \
				 CONFIG_SYS_POST_WATCHDOG	| \
				 CONFIG_SYS_POST_DSP		| \
				 CONFIG_SYS_POST_BSPEC1		| \
				 CONFIG_SYS_POST_BSPEC2		| \
				 CONFIG_SYS_POST_BSPEC3		| \
				 CONFIG_SYS_POST_BSPEC4		| \
				 CONFIG_SYS_POST_BSPEC5)

/* Define here the base-addresses of the UARTs to test in POST */
#define CONFIG_SYS_POST_UART_TABLE	{ CONFIG_SYS_NS16550_COM1, \
			CONFIG_SYS_NS16550_COM2 }

#define CONFIG_POST_UART  {				\
	"UART test",					\
	"uart",						\
	"This test verifies the UART operation.",	\
	POST_RAM | POST_SLOWTEST | POST_ALWAYS | POST_MANUAL,	\
	&uart_post_test,				\
	NULL,						\
	NULL,						\
	CONFIG_SYS_POST_UART				\
	}

#define CONFIG_POST_WATCHDOG  {				\
	"Watchdog timer test",				\
	"watchdog",					\
	"This test checks the watchdog timer.",		\
	POST_RAM | POST_POWERON | POST_SLOWTEST | POST_MANUAL | POST_REBOOT, \
	&lwmon5_watchdog_post_test,			\
	NULL,						\
	NULL,						\
	CONFIG_SYS_POST_WATCHDOG			\
	}

#define CONFIG_POST_BSPEC1    {				\
	"dsPIC init test",				\
	"dspic_init",					\
	"This test returns result of dsPIC READY test run earlier.",	\
	POST_RAM | POST_ALWAYS,				\
	&dspic_init_post_test,				\
	NULL,						\
	NULL,						\
	CONFIG_SYS_POST_BSPEC1				\
	}

#define CONFIG_POST_BSPEC2    {				\
	"dsPIC test",					\
	"dspic",					\
	"This test gets result of dsPIC POST and dsPIC version.",	\
	POST_RAM | POST_ALWAYS,				\
	&dspic_post_test,				\
	NULL,						\
	NULL,						\
	CONFIG_SYS_POST_BSPEC2				\
	}

#define CONFIG_POST_BSPEC3    {				\
	"FPGA test",					\
	"fpga",						\
	"This test checks FPGA registers and memory.",	\
	POST_RAM | POST_ALWAYS | POST_MANUAL,		\
	&fpga_post_test,				\
	NULL,						\
	NULL,						\
	CONFIG_SYS_POST_BSPEC3				\
	}

#define CONFIG_POST_BSPEC4    {				\
	"GDC test",					\
	"gdc",						\
	"This test checks GDC registers and memory.",	\
	POST_RAM | POST_ALWAYS | POST_MANUAL,\
	&gdc_post_test,					\
	NULL,						\
	NULL,						\
	CONFIG_SYS_POST_BSPEC4				\
	}

#define CONFIG_POST_BSPEC5    {				\
	"SYSMON1 test",					\
	"sysmon1",					\
	"This test checks GPIO_62_EPX pin indicating power failure.",	\
	POST_RAM | POST_MANUAL | POST_NORMAL | POST_SLOWTEST,	\
	&sysmon1_post_test,				\
	NULL,						\
	NULL,						\
	CONFIG_SYS_POST_BSPEC5				\
	}

#define CONFIG_SYS_POST_CACHE_ADDR	0x7fff0000 /* free virtual address	*/
#define CONFIG_LOGBUFFER
/* Reserve GPT0_COMP1-COMP5 for logbuffer header */
#define CONFIG_ALT_LH_ADDR	(CONFIG_SYS_PERIPHERAL_BASE + GPT0_COMP1)
#define CONFIG_ALT_LB_ADDR	(CONFIG_SYS_OCM_BASE)
#define CONFIG_SYS_CONSOLE_IS_IN_ENV /* Otherwise it catches logbuffer as output */
#endif

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		100000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F

#define CONFIG_SYS_I2C_RTC_ADDR	0x51	/* RTC				*/
#define CONFIG_SYS_I2C_EEPROM_CPU_ADDR	0x52	/* EEPROM          (CPU Modul)	*/
#define CONFIG_SYS_I2C_EEPROM_MB_ADDR	0x53	/* EEPROM AT24C128 (MainBoard)	*/
#define CONFIG_SYS_I2C_DSPIC_ADDR	0x54	/* dsPIC   			*/
#define CONFIG_SYS_I2C_DSPIC_2_ADDR	0x55	/* dsPIC			*/
#define CONFIG_SYS_I2C_DSPIC_KEYB_ADDR	0x56	/* dsPIC			*/
#define CONFIG_SYS_I2C_DSPIC_IO_ADDR	0x57	/* dsPIC			*/

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2	/* Bytes of address		*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 6	/* The Atmel AT24C128 has	*/
					/* 64 byte page write mode using*/
					/* last 6 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE

#define CONFIG_RTC_PCF8563			/* enable Philips PCF8563 RTC	*/
#define CONFIG_SYS_I2C_RTC_ADDR		0x51	/* Philips PCF8563 RTC address	*/
#define CONFIG_SYS_I2C_KEYBD_ADDR	0x56	/* PIC LWE keyboard		*/
#define CONFIG_SYS_I2C_DSPIC_IO_ADDR	0x57	/* PIC I/O addr               */

#define CONFIG_SYS_POST_I2C_ADDRS	{CONFIG_SYS_I2C_RTC_ADDR,	\
					 CONFIG_SYS_I2C_EEPROM_CPU_ADDR,\
					 CONFIG_SYS_I2C_EEPROM_MB_ADDR,	\
					 CONFIG_SYS_I2C_DSPIC_ADDR,	\
					 CONFIG_SYS_I2C_DSPIC_2_ADDR,	\
					 CONFIG_SYS_I2C_DSPIC_KEYB_ADDR,\
					 CONFIG_SYS_I2C_DSPIC_IO_ADDR }

/*
 * Pass open firmware flat tree
 */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
/* Update size in "reg" property of NOR FLASH device tree nodes */
#define CONFIG_FDT_FIXUP_NOR_FLASH_SIZE

#define CONFIG_FIT			/* enable FIT image support	*/

#define	CONFIG_POST_KEY_MAGIC	"3C+3E"	/* press F3 + F5 keys to force POST */

#define	CONFIG_PREBOOT		"setenv bootdelay 15"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"hostname=lwmon5\0"						\
	"netdev=eth0\0"							\
	"unlock=yes\0"							\
	"logversion=2\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS1,${baudrate}\0"\
	"addmisc=setenv bootargs ${bootargs} rtc-pcf8563.probe=0,0x51\0"\
	"flash_nfs=run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty addmisc;"			\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};"				\
		"run nfsargs addip addtty addmisc;bootm\0"		\
	"rootpath=/opt/eldk/ppc_4xxFP\0"				\
	"bootfile=/tftpboot/lwmon5/uImage\0"				\
	"kernel_addr=FC000000\0"					\
	"ramdisk_addr=FC180000\0"					\
	"load=tftp 200000 /tftpboot/${hostname}/u-boot.bin\0"		\
	"update=protect off FFF80000 FFFFFFFF;era FFF80000 FFFFFFFF;"	\
		"cp.b 200000 FFF80000 80000\0"			        \
	"upd=run load update\0"						\
	"lwe_env=tftp 200000 /tftpboot.dev/lwmon5/env_uboot.bin;"	\
		"autoscr 200000\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		3	/* PHY address, See schematics	*/

#define CONFIG_PHY_RESET        1	/* reset phy upon startup         */
#define CONFIG_PHY_RESET_DELAY	300

#define CONFIG_HAS_ETH0
#define CONFIG_SYS_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_PHY1_ADDR	1

/* Video console */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_MB862xx
#define CONFIG_VIDEO_MB862xx_ACCEL
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_CONSOLE_EXTRA_INFO
#define VIDEO_FB_16BPP_PIXEL_SWAP
#define VIDEO_FB_16BPP_WORD_SWAP

#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_SPLASH_SCREEN

#ifndef CONFIG_LCD4_LWMON5
/*
 * USB/EHCI
 */
#define CONFIG_USB_EHCI			/* Enable EHCI USB support	*/
#define CONFIG_USB_EHCI_PPC4XX		/* on PPC4xx platform		*/
#define CONFIG_SYS_PPC4XX_USB_ADDR	0xe0000300
#define CONFIG_EHCI_MMIO_BIG_ENDIAN
#define CONFIG_EHCI_DESC_BIG_ENDIAN
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET /* re-init HCD after CMD_RESET */
#define CONFIG_USB_STORAGE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION
#endif

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
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM

#ifdef CONFIG_VIDEO
#define CONFIG_CMD_BMP
#endif

#ifndef CONFIG_LCD4_LWMON5
#ifdef CONFIG_440EPX
#define CONFIG_CMD_USB
#endif
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SUPPORT_VFAT

#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/

#define CONFIG_SYS_HUSH_PARSER		1	/* Use the HUSH parser		*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	        16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	        CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000  /* default load address	*/
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* don't print console @ startup*/

#ifndef CONFIG_LCD4_LWMON5
#ifndef DEBUG
#define CONFIG_HW_WATCHDOG	1	/* Use external HW-Watchdog	*/
#endif
#define CONFIG_WD_PERIOD	40000	/* in usec */
#define CONFIG_WD_MAX_RATE	66600	/* in ticks */
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the 40x Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(16 << 20) /* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTM_LEN		(16 << 20) /* Increase max gunzip size */

/*
 * External Bus Controller (EBC) Setup
 */
#define CONFIG_SYS_FLASH		CONFIG_SYS_FLASH_BASE

/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CONFIG_SYS_EBC_PB0AP		0x03000280
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH | 0xfc000)

/* Memory Bank 1 (Lime) initialization						*/
#define CONFIG_SYS_EBC_PB1AP		0x01004380
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_LIME_BASE_0 | 0xbc000)

/* Memory Bank 2 (FPGA) initialization						*/
#define CONFIG_SYS_EBC_PB2AP		0x01004400
#define CONFIG_SYS_EBC_PB2CR		(CONFIG_SYS_FPGA_BASE_0 | 0x1c000)

/* Memory Bank 3 (FPGA2) initialization						*/
#define CONFIG_SYS_EBC_PB3AP		0x01004400
#define CONFIG_SYS_EBC_PB3CR		(CONFIG_SYS_FPGA_BASE_1 | 0x1c000)

#define CONFIG_SYS_EBC_CFG		0xb8400000

/*
 * Graphics (Fujitsu Lime)
 */
/* SDRAM Clock frequency adjustment register */
#define CONFIG_SYS_LIME_SDRAM_CLOCK	0xC1FC0038
#if 1 /* 133MHz is not tested enough, use 100MHz for now */
/* Lime Clock frequency is to set 100MHz */
#define CONFIG_SYS_LIME_CLOCK_100MHZ	0x00000
#else
/* Lime Clock frequency for 133MHz */
#define CONFIG_SYS_LIME_CLOCK_133MHZ	0x10000
#endif

/* SDRAM Parameter register */
#define CONFIG_SYS_LIME_MMR		0xC1FCFFFC
/*
 * SDRAM parameter value; was 0x414FB7F2, caused several vertical bars
 * and pixel flare on display when 133MHz was configured. According to
 * SDRAM chip datasheet CAS Latency is 3 for 133MHz and -75 Speed
 * Grade
 */
#ifdef CONFIG_SYS_LIME_CLOCK_133MHZ
#define CONFIG_SYS_MB862xx_MMR	0x414FB7F3
#define CONFIG_SYS_MB862xx_CCF	CONFIG_SYS_LIME_CLOCK_133MHZ
#else
#define CONFIG_SYS_MB862xx_MMR	0x414FB7F2
#define CONFIG_SYS_MB862xx_CCF	CONFIG_SYS_LIME_CLOCK_100MHZ
#endif

/*
 * GPIO Setup
 */
#define CONFIG_SYS_GPIO_PHY1_RST	12
#define CONFIG_SYS_GPIO_FLASH_WP	14
#define CONFIG_SYS_GPIO_PHY0_RST	22
#define CONFIG_SYS_GPIO_PERM_VOLT_FEED	49
#define CONFIG_SYS_GPIO_DSPIC_READY	51
#define CONFIG_SYS_GPIO_CAN_ENABLE	53
#define CONFIG_SYS_GPIO_LSB_ENABLE	54
#define CONFIG_SYS_GPIO_EEPROM_EXT_WP	55
#define CONFIG_SYS_GPIO_HIGHSIDE	56
#define CONFIG_SYS_GPIO_EEPROM_INT_WP	57
#define CONFIG_SYS_GPIO_BOARD_RESET	58
#define CONFIG_SYS_GPIO_LIME_S		59
#define CONFIG_SYS_GPIO_LIME_RST	60
#define CONFIG_SYS_GPIO_SYSMON_STATUS	62
#define CONFIG_SYS_GPIO_WATCHDOG	63

/* On LCD4, GPIO49 has to be configured to 0 instead of 1 */
#ifdef CONFIG_LCD4_LWMON5
#define GPIO49_VAL	0
#else
#define GPIO49_VAL	1
#endif

/*
 * PPC440 GPIO Configuration
 */
#define CONFIG_SYS_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO0	EBC_ADDR(7)	DMA_REQ(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO1	EBC_ADDR(6)	DMA_ACK(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO2	EBC_ADDR(5)	DMA_EOT/TC(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO3	EBC_ADDR(4)	DMA_REQ(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO4	EBC_ADDR(3)	DMA_ACK(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO5	EBC_ADDR(2)	DMA_EOT/TC(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO6	EBC_CS_N(1)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO7	EBC_CS_N(2)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	EBC_CS_N(3)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	EBC_CS_N(4)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 EBC_CS_N(5)			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO11 EBC_BUS_ERR			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO12				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO13				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO14				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO15				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO16 GMCTxD(4)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 GMCTxD(5)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 GMCTxD(6)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO19 GMCTxD(7)			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO20 RejectPkt0			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO21 RejectPkt1			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO22				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO23 SCPD0				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO24 GMCTxD(2)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO25 GMCTxD(3)			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO26				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO27 EXT_EBC_REQ	USB2D_RXERROR	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO28		USB2D_TXVALID	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO29 EBC_EXT_HDLA	USB2D_PAD_SUSPNDM */	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO30 EBC_EXT_ACK	USB2D_XCVRSELECT*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO31 EBC_EXR_BUSREQ	USB2D_TERMSELECT*/	\
},											\
{											\
/* GPIO Core 1 */									\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO32 USB2D_OPMODE0	EBC_DATA(2)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO33 USB2D_OPMODE1	EBC_DATA(3)	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_0}, /* GPIO34 UART0_DCD_N	UART1_DSR_CTS_N	UART2_SOUT*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO35 UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO36 UART0_8PIN_CTS_N EBC_DATA(0)	UART3_SIN*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_0}, /* GPIO37 UART0_RTS_N	EBC_DATA(1)	UART3_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_0}, /* GPIO38 UART0_DTR_N	UART1_SOUT	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO39 UART0_RI_N	UART1_SIN	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO40 UIC_IRQ(0)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO41 UIC_IRQ(1)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO42 UIC_IRQ(2)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO43 UIC_IRQ(3)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO44 UIC_IRQ(4)	DMA_ACK(1)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO45 UIC_IRQ(6)	DMA_EOT/TC(1)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO46 UIC_IRQ(7)	DMA_REQ(0)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO47 UIC_IRQ(8)	DMA_ACK(0)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO48 UIC_IRQ(9)	DMA_EOT/TC(0)	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO49_VAL}, /* GPIO49  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL , GPIO_OUT_0}, /* GPIO50  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO51  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO52  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO53  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO54  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO55  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO56  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO61  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO62  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO63  Unselect via TraceSelect Bit	*/	\
}											\
}

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/*
 * SPL related defines
 */
#ifdef CONFIG_LCD4_LWMON5
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_TEXT_BASE		0xffff0000 /* last 64 KiB for SPL */
#define CONFIG_SYS_SPL_MAX_LEN		(64 << 10)
#define CONFIG_UBOOT_PAD_TO		458752	/* decimal for 'dd' */
#define	CONFIG_SPL_START_S_PATH	"arch/powerpc/cpu/ppc4xx"
#define CONFIG_SPL_LDSCRIPT	"arch/powerpc/cpu/ppc4xx/u-boot-spl.lds"
#define CONFIG_SPL_LIBCOMMON_SUPPORT	/* image.c */
#define CONFIG_SPL_LIBGENERIC_SUPPORT	/* string.c */
#define CONFIG_SPL_SERIAL_SUPPORT

/* Place BSS for SPL near end of SDRAM */
#define CONFIG_SPL_BSS_START_ADDR	((256 - 1) << 20)
#define CONFIG_SPL_BSS_MAX_SIZE		(64 << 10)

#define CONFIG_SPL_OS_BOOT
/* Place patched DT blob (fdt) at this address */
#define CONFIG_SYS_SPL_ARGS_ADDR	0x01800000

#define CONFIG_SPL_TARGET		"u-boot-img-spl-at-end.bin"

/* Settings for real U-Boot to be loaded from NOR flash */
#define CONFIG_SYS_UBOOT_BASE		(-CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_UBOOT_START		0x01002100

#define CONFIG_SYS_OS_BASE		0xf8000000
#define CONFIG_SYS_FDT_BASE		0xf87c0000
#endif

#endif	/* __CONFIG_H */
