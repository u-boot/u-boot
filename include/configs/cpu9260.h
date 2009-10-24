/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 *
 * (C) Copyright 2009
 * Eric Benard <eric@eukrea.com>
 *
 * Configuration settings for the Eukrea CPU9260 board.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_DISPLAY_CPUINFO	1

#define AT91_MAIN_CLOCK		18432000
#define CONFIG_SYS_HZ		1000

#define CONFIG_ARM926EJS	1

#if defined(CONFIG_CPU9260_128M) || defined(CONFIG_CPU9260)
#define CONFIG_CPU9260		1
#elif defined(CONFIG_CPU9G20_128M) || defined(CONFIG_CPU9G20)
#define CONFIG_CPU9G20		1
#endif

#if defined(CONFIG_CPU9G20)
#define CONFIG_AT91SAM9G20	1
#elif defined(CONFIG_CPU9260)
#define CONFIG_AT91SAM9260	1
#else
#error "Unknown board"
#endif

#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ

#define CONFIG_CMDLINE_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS 	1
#define CONFIG_INITRD_TAG		1

/* clocks */
#if defined(CONFIG_CPU9G20)
#define MASTER_PLL_DIV		0x01
#define MASTER_PLL_MUL		0x2B
#elif defined(CONFIG_CPU9260)
#define MASTER_PLL_DIV		0x09
#define MASTER_PLL_MUL		0x61
#endif

/* CKGR_MOR - enable main osc. */
#define CONFIG_SYS_MOR_VAL						\
		(AT91_PMC_MOSCEN |					\
		 (255 << 8))		/* Main Oscillator Start-up Time */
#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_PLLAR_VAL						\
		(AT91_PMC_PLLA_WR_ERRATA | /* Bit 29 must be 1 when prog */ \
		 ((MASTER_PLL_MUL - 1) << 16) | (MASTER_PLL_DIV))
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_PLLAR_VAL						\
		(AT91_PMC_PLLA_WR_ERRATA | /* Bit 29 must be 1 when prog */ \
		 AT91_PMC_OUT |						\
		 ((MASTER_PLL_MUL - 1) << 16) | (MASTER_PLL_DIV))
#endif

#if defined(CONFIG_CPU9G20)
#define	CONFIG_SYS_MCKR1_VAL		\
		(AT91_PMC_CSS_PLLA |	\
		 AT91_PMC_PRES_1 |	\
		 AT91SAM9_PMC_MDIV_6 |	\
		 AT91_PMC_PDIV_2)
#define	CONFIG_SYS_MCKR2_VAL		\
		CONFIG_SYS_MCKR1_VAL
#elif defined(CONFIG_CPU9260)
#define	CONFIG_SYS_MCKR1_VAL		\
		(AT91_PMC_CSS_SLOW |	\
		 AT91_PMC_PRES_1 |	\
		 AT91SAM9_PMC_MDIV_2 |	\
		 AT91_PMC_PDIV_1)
#define	CONFIG_SYS_MCKR2_VAL		\
		(AT91_PMC_CSS_PLLA |	\
		 AT91_PMC_PRES_1 |	\
		 AT91SAM9_PMC_MDIV_2 |	\
		 AT91_PMC_PDIV_1)
#endif

/* define PDC[31:16] as DATA[31:16] */
#define CONFIG_SYS_PIOC_PDR_VAL1	0xFFFF0000
/* no pull-up for D[31:16] */
#define CONFIG_SYS_PIOC_PPUDR_VAL	0xFFFF0000

/* EBI_CSA, 3.3V, no pull-ups for D[15:0], CS1 SDRAM, CS3 NAND Flash */
#define CONFIG_SYS_MATRIX_EBICSA_VAL		\
       (AT91_MATRIX_DBPUC | AT91_MATRIX_CS1A_SDRAMC |\
       AT91_MATRIX_CS3A_SMC_SMARTMEDIA | AT91_MATRIX_VDDIOMSEL)

/* SDRAM */
/* SDRAMC_MR Mode register */
#define CONFIG_SYS_SDRC_MR_VAL1		AT91_SDRAMC_MODE_NORMAL
/* SDRAMC_TR - Refresh Timer register */
#define CONFIG_SYS_SDRC_TR_VAL1		0x287
/* SDRAMC_CR - Configuration register*/
#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_SDRC_CR_VAL_64MB					\
		(AT91_SDRAMC_NC_9 |					\
		 AT91_SDRAMC_NR_13 |					\
		 AT91_SDRAMC_NB_4 |					\
		 AT91_SDRAMC_CAS_2 |					\
		 AT91_SDRAMC_DBW_32 |					\
		 (2 <<  8) |	/* Write Recovery Delay */		\
		 (9 << 12) |	/* Row Cycle Delay */			\
		 (3 << 16) |	/* Row Precharge Delay */		\
		 (3 << 20) |	/* Row to Column Delay */		\
		 (6 << 24) |	/* Active to Precharge Delay */		\
		 (10 << 28))	/* Exit Self Refresh to Active Delay */

#define CONFIG_SYS_SDRC_CR_VAL_128MB					\
		(AT91_SDRAMC_NC_10 |					\
		 AT91_SDRAMC_NR_13 |					\
		 AT91_SDRAMC_NB_4 |					\
		 AT91_SDRAMC_CAS_2 |					\
		 AT91_SDRAMC_DBW_32 |					\
		 (2 <<  8) |	/* Write Recovery Delay */		\
		 (9 << 12) |	/* Row Cycle Delay */			\
		 (3 << 16) |	/* Row Precharge Delay */		\
		 (3 << 20) |	/* Row to Column Delay */		\
		 (6 << 24) |	/* Active to Precharge Delay */		\
		 (10 << 28))	/* Exit Self Refresh to Active Delay */
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_SDRC_CR_VAL_64MB					\
		(AT91_SDRAMC_NC_9 |					\
		 AT91_SDRAMC_NR_13 |					\
		 AT91_SDRAMC_NB_4 |					\
		 AT91_SDRAMC_CAS_2 |					\
		 AT91_SDRAMC_DBW_32 |					\
		 (2 <<  8) |	/* Write Recovery Delay */		\
		 (7 << 12) |	/* Row Cycle Delay */			\
		 (2 << 16) |	/* Row Precharge Delay */		\
		 (2 << 20) |	/* Row to Column Delay */		\
		 (5 << 24) |	/* Active to Precharge Delay */		\
		 (8 << 28))	/* Exit Self Refresh to Active Delay */

#define CONFIG_SYS_SDRC_CR_VAL_128MB					\
		(AT91_SDRAMC_NC_10 |					\
		 AT91_SDRAMC_NR_13 |					\
		 AT91_SDRAMC_NB_4 |					\
		 AT91_SDRAMC_CAS_2 |					\
		 AT91_SDRAMC_DBW_32 |					\
		 (2 <<  8) |	/* Write Recovery Delay */		\
		 (7 << 12) |	/* Row Cycle Delay */			\
		 (2 << 16) |	/* Row Precharge Delay */		\
		 (2 << 20) |	/* Row to Column Delay */		\
		 (5 << 24) |	/* Active to Precharge Delay */		\
		 (8 << 28))	/* Exit Self Refresh to Active Delay */
#endif

/* Memory Device Register -> SDRAM */
#define CONFIG_SYS_SDRC_MDR_VAL		AT91_SDRAMC_MD_SDRAM
#define CONFIG_SYS_SDRC_MR_VAL2		AT91_SDRAMC_MODE_PRECHARGE
#define CONFIG_SYS_SDRAM_VAL1		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL3		AT91_SDRAMC_MODE_REFRESH
#define CONFIG_SYS_SDRAM_VAL2		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL3		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL4		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL5		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL6		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL7		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL8		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL9		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL4		AT91_SDRAMC_MODE_LMR
#define CONFIG_SYS_SDRAM_VAL10		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL5		AT91_SDRAMC_MODE_NORMAL
#define CONFIG_SYS_SDRAM_VAL11		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_TR_VAL2		1200		/* SDRAM_TR */
#define CONFIG_SYS_SDRAM_VAL12		0		/* SDRAM_BASE */

/* setup SMC0, CS0 (NOR Flash) - 16-bit */
#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_SMC0_SETUP0_VAL					\
		(AT91_SMC_NWESETUP_(0) | AT91_SMC_NCS_WRSETUP_(0) |	\
		 AT91_SMC_NRDSETUP_(0) | AT91_SMC_NCS_RDSETUP_(0))
#define CONFIG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_NWEPULSE_(8) | AT91_SMC_NCS_WRPULSE_(8) |	\
		 AT91_SMC_NRDPULSE_(14) | AT91_SMC_NCS_RDPULSE_(14))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_NWECYCLE_(8) | AT91_SMC_NRDCYCLE_(14))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_READMODE | AT91_SMC_WRITEMODE |	\
		 AT91_SMC_DBW_16 |				\
		 AT91_SMC_TDFMODE |				\
		 AT91_SMC_TDF_(3))
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_SMC0_SETUP0_VAL					\
		(AT91_SMC_NWESETUP_(0) | AT91_SMC_NCS_WRSETUP_(0) |	\
		 AT91_SMC_NRDSETUP_(0) | AT91_SMC_NCS_RDSETUP_(0))
#define CONFIG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_NWEPULSE_(6) | AT91_SMC_NCS_WRPULSE_(6) |	\
		 AT91_SMC_NRDPULSE_(10) | AT91_SMC_NCS_RDPULSE_(10))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_NWECYCLE_(6) | AT91_SMC_NRDCYCLE_(10))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_READMODE | AT91_SMC_WRITEMODE |	\
		 AT91_SMC_DBW_16 |				\
		 AT91_SMC_TDFMODE |				\
		 AT91_SMC_TDF_(2))
#endif

/* user reset enable */
#define CONFIG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_PROCRST |		\
		AT91_RSTC_RSTTYP_WAKEUP |	\
		AT91_RSTC_RSTTYP_WATCHDOG)

/* Disable Watchdog */
#define CONFIG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_WDIDLEHLT | AT91_WDT_WDDBGHLT |	\
		 AT91_WDT_WDV |					\
		 AT91_WDT_WDDIS |				\
		 AT91_WDT_WDD)

/*
 * Hardware drivers
 */
#define CONFIG_ATMEL_USART	1
#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2
#define CONFIG_USART3		1	/* USART 3 is DBGU */

#define CONFIG_BOOTDELAY	3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE	1
#define CONFIG_BOOTP_BOOTPATH		1
#define CONFIG_BOOTP_GATEWAY		1
#define CONFIG_BOOTP_HOSTNAME		1

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_IMLS

#define CONFIG_CMD_PING		1
#define CONFIG_CMD_DHCP		1
#define CONFIG_CMD_NAND		1
#define CONFIG_CMD_USB		1
#define CONFIG_CMD_FAT		1

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM		0x20000000
#if defined(CONFIG_CPU9260_128M) || defined(CONFIG_CPU9G20_128M)
#define PHYS_SDRAM_SIZE		0x08000000	/* 128 MB */
#define CONFIG_SYS_SDRC_CR_VAL	CONFIG_SYS_SDRC_CR_VAL_128MB
#else
#define PHYS_SDRAM_SIZE		0x04000000	/* 64 MB */
#define CONFIG_SYS_SDRC_CR_VAL	CONFIG_SYS_SDRC_CR_VAL_64MB
#endif

/* NAND flash */
#define CONFIG_NAND_ATMEL			1
#define NAND_MAX_CHIPS				1
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8			1
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC13
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_64BIT_VSPRINTF		/* needed for nand_util.c */

/* NOR flash */
#define CONFIG_SYS_FLASH_CFI			1
#define CONFIG_FLASH_CFI_DRIVER			1
#define PHYS_FLASH_1				0x10000000
#define PHYS_FLASH_2				0x12000000
#define CONFIG_SYS_FLASH_BANKS_LIST		\
		{ PHYS_FLASH_1, PHYS_FLASH_2 }
#define CONFIG_SYS_FLASH_BASE			PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT		(255+4)
#define CONFIG_SYS_MAX_FLASH_BANKS		2
#define CONFIG_SYS_FLASH_CFI_WIDTH		FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_EMPTY_INFO		1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
#define CONFIG_SYS_FLASH_PROTECTION		1
#define CONFIG_SYS_MONITOR_BASE			PHYS_FLASH_1

/* Ethernet */
#define CONFIG_MACB				1
#define CONFIG_RMII				1
#define CONFIG_RESET_PHY_R			1
#define CONFIG_NET_MULTI			1
#define CONFIG_NET_RETRY_COUNT			20
#define CONFIG_MACB_SEARCH_PHY			1

/* LEDS */
/* Status LED */
#define CONFIG_STATUS_LED			1 /* Status LED enabled	*/
#define CONFIG_BOARD_SPECIFIC_LED		1
#define STATUS_LED_RED				0
#define STATUS_LED_GREEN			1
#define STATUS_LED_YELLOW			2
#define STATUS_LED_BLUE				3
/* Red */
#define STATUS_LED_BIT				STATUS_LED_RED
#define STATUS_LED_STATE			STATUS_LED_OFF
#define STATUS_LED_PERIOD			(CONFIG_SYS_HZ / 2)
/* Green */
#define STATUS_LED_BIT1				STATUS_LED_GREEN
#define STATUS_LED_STATE1			STATUS_LED_OFF
#define STATUS_LED_PERIOD1			(CONFIG_SYS_HZ / 2)
/* Yellow */
#define STATUS_LED_BIT2				STATUS_LED_YELLOW
#define STATUS_LED_STATE2			STATUS_LED_OFF
#define STATUS_LED_PERIOD2			(CONFIG_SYS_HZ / 2)
/* Blue */
#define STATUS_LED_BIT3				STATUS_LED_BLUE
#define STATUS_LED_STATE3			STATUS_LED_ON
#define STATUS_LED_PERIOD3			(CONFIG_SYS_HZ / 2)
/* Optional value */
#define STATUS_LED_BOOT				STATUS_LED_BIT

#define CONFIG_RED_LED				AT91_PIN_PC11
#define CONFIG_GREEN_LED			AT91_PIN_PC12
#define CONFIG_YELLOW_LED			AT91_PIN_PC7
#define CONFIG_BLUE_LED				AT91_PIN_PC9

/* USB */
#define CONFIG_USB_ATMEL			1
#define CONFIG_USB_OHCI_NEW			1
#define CONFIG_DOS_PARTITION			1
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE			1

#define CONFIG_SYS_LOAD_ADDR			0x21000000

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			0x21e00000

#undef CONFIG_SYS_USE_NANDFLASH
#define CONFIG_SYS_USE_FLASH			1

#if defined(CONFIG_SYS_USE_FLASH)
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_SECT_SIZE		0x20000
#define	CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_OVERWRITE		1

#define CONFIG_BOOTCOMMAND		"run flashboot"

#define MTDIDS_DEFAULT	 	"nor0=physmap-flash.0,nand0=atmel_nand"
#define MTDPARTS_DEFAULT		\
	"mtdparts=physmap-flash.0:"	\
		"256k(u-boot)ro,"	\
		"128k(u-boot-env)ro,"	\
		"1792k(kernel),"	\
		"-(rootfs);"		\
	"atmel_nand:-(nand)"

#define CONFIG_BOOTARGS "root=/dev/mtdblock3 rootfstype=jffs2 "

#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_BASEDIR	"cpu9G20"
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_BASEDIR	"cpu9260"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"mtdids=" MTDIDS_DEFAULT "\0"				\
	"mtdparts=" MTDPARTS_DEFAULT "\0"			\
	"partition=nand0,0\0"					\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"	\
	"ramboot=tftpboot 0x22000000 cpu9260/uImage;"		\
		"run ramargs;bootm 22000000\0"			\
	"flashboot=run ramargs;bootm 0x10060000\0"		\
	"basedir=" CONFIG_SYS_BASEDIR "\0"			\
	"updtub=tftp 0x24000000 $(basedir)/u-boot.bin;protect "	\
		"off 0x10000000 0x1003ffff;erase 0x10000000 "	\
		"0x1003ffff;cp.b 0x24000000 0x10000000 "	\
		"$(filesize)\0" \
	"updtui=tftp 0x24000000 $(basedir)/uImage;protect off"	\
		" 0x10060000 0x1021ffff;erase 0x10060000 "	\
		"0x1021ffff;cp.b 0x24000000 0x10060000 "	\
		"$(filesize)\0" \
	"updtrfs=tftp 0x24000000 $(basedir)/rootfs.jffs2; "	\
		"protect off 0x10220000 0x13ffffff;erase "	\
		"0x10220000 0x13ffffff;cp.b 0x24000000 "	\
		"0x10220000 $(filesize)\0" \
	""
#endif

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_PROMPT		"CPU9G20=> "
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_PROMPT		"CPU9260=> "
#endif
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		\
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING		1
#define CONFIG_SILENT_CONSOLE		1
#define CONFIG_NETCONSOLE		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		\
		ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128

#define CONFIG_STACKSIZE		(32 * 1024)

#if defined(CONFIG_USE_IRQ)
#error CONFIG_USE_IRQ not supported
#endif

#endif
