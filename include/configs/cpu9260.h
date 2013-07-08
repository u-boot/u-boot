/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 *
 * (C) Copyright 2009
 * Eric Benard <eric@eukrea.com>
 *
 * Configuration settings for the Eukrea CPU9260 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* to be removed once maemory-map.h is fixed */
#define AT91_BASE_SYS	0xffffe800
#define AT91_DBGU	(0xfffff200 - AT91_BASE_SYS)

#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000
#define CONFIG_SYS_HZ		1000
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768

#if defined(CONFIG_CPU9G20)
#define CONFIG_AT91SAM9G20
#elif defined(CONFIG_CPU9260)
#define CONFIG_AT91SAM9260
#else
#error "Unknown board"
#endif

#include <asm/arch/hardware.h>

#define CONFIG_AT91FAMILY
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#if defined(CONFIG_NANDBOOT)
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE           0x23f00000
#else
#define CONFIG_SYS_TEXT_BASE           0x00000000
#endif

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
		(AT91_MATRIX_CSA_DBPUC | AT91_MATRIX_CSA_EBI_CS1A | \
		AT91_MATRIX_CSA_EBI_CS3A | AT91_MATRIX_CSA_VDDIOMSEL_3_3V)

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
		(AT91_SMC_SETUP_NWE(0) | AT91_SMC_SETUP_NCS_WR(0) |	\
		 AT91_SMC_SETUP_NRD(0) | AT91_SMC_SETUP_NCS_RD(0))
#define CONFIG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_PULSE_NWE(8) | AT91_SMC_PULSE_NCS_WR(8) |	\
		 AT91_SMC_PULSE_NRD(14) | AT91_SMC_PULSE_NCS_RD(14))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_CYCLE_NWE(8) | AT91_SMC_CYCLE_NRD(14))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |	\
		 AT91_SMC_MODE_DBW_16 |				\
		 AT91_SMC_MODE_TDF |				\
		 AT91_SMC_MODE_TDF_CYCLE(3))
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_SMC0_SETUP0_VAL					\
		(AT91_SMC_SETUP_NWE(0) | AT91_SMC_SETUP_NCS_WR(0) |	\
		 AT91_SMC_SETUP_NRD(0) | AT91_SMC_SETUP_NCS_RD(0))
#define CONFIG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_PULSE_NWE(6) | AT91_SMC_PULSE_NCS_WR(6) |	\
		 AT91_SMC_PULSE_NRD(10) | AT91_SMC_PULSE_NCS_RD(10))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_CYCLE_NWE(6) | AT91_SMC_CYCLE_NRD(10))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |	\
		 AT91_SMC_MODE_DBW_16 |				\
		 AT91_SMC_MODE_TDF |				\
		 AT91_SMC_MODE_TDF_CYCLE(2))
#endif

/* user reset enable */
#define CONFIG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_CR_PROCRST |		\
		AT91_RSTC_MR_ERSTL(1) |	\
		AT91_RSTC_MR_ERSTL(2))

/* Disable Watchdog */
#define CONFIG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_MR_WDIDLEHLT | AT91_WDT_MR_WDDBGHLT |	\
		 AT91_WDT_MR_WDV(0xfff) |			\
		 AT91_WDT_MR_WDDIS |				\
		 AT91_WDT_MR_WDD(0xfff))

/*
 * Hardware drivers
 */
#define CONFIG_AT91SAM9_WATCHDOG
#define CONFIG_AT91_GPIO
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE	ATMEL_BASE_DBGU
#define CONFIG_USART_ID		ATMEL_ID_SYS

#define CONFIG_BOOTDELAY	3

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
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_IMLS

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NAND
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MII

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS	1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#if defined(CONFIG_CPU9260_128M) || defined(CONFIG_CPU9G20_128M)
#define CONFIG_SYS_SDRAM_SIZE		(128 * 1024 * 1024)
#define CONFIG_SYS_SDRC_CR_VAL	CONFIG_SYS_SDRC_CR_VAL_128MB
#else
#define CONFIG_SYS_SDRAM_SIZE		(64 * 1024 * 1024)
#define CONFIG_SYS_SDRC_CR_VAL	CONFIG_SYS_SDRC_CR_VAL_64MB
#endif

/* NAND flash */
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8			1
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIO_PORTC, 13
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIO_PORTC, 14
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)

/* NOR flash */
#if defined(CONFIG_NANDBOOT)
#define CONFIG_SYS_NO_FLASH
#else
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define PHYS_FLASH_1				0x10000000
#define PHYS_FLASH_2				0x12000000
#define CONFIG_SYS_FLASH_BANKS_LIST		\
		{ PHYS_FLASH_1, PHYS_FLASH_2 }
#define CONFIG_SYS_FLASH_BASE			PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT		(255+4)
#define CONFIG_SYS_MAX_FLASH_BANKS		2
#define CONFIG_SYS_FLASH_CFI_WIDTH		FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MONITOR_BASE			PHYS_FLASH_1
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT			20
#define CONFIG_MACB_SEARCH_PHY

/* LEDS */
/* Status LED */
#define CONFIG_STATUS_LED
#define CONFIG_BOARD_SPECIFIC_LED
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

#define CONFIG_RED_LED				AT91_PIO_PORTC, 11
#define CONFIG_GREEN_LED			AT91_PIO_PORTC, 12
#define CONFIG_YELLOW_LED			AT91_PIO_PORTC, 7
#define CONFIG_BLUE_LED				AT91_PIO_PORTC, 9

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000
#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9g20"
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9260"
#endif
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE

#define CONFIG_SYS_LOAD_ADDR			0x21000000
#define CONFIG_LOADADDR				CONFIG_SYS_LOAD_ADDR

#define CONFIG_SYS_MEMTEST_START		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			\
	(CONFIG_SYS_MEMTEST_START + CONFIG_SYS_SDRAM_SIZE - 512 * 1024)

#if defined(CONFIG_NANDBOOT)
#define CONFIG_SYS_USE_NANDFLASH
#undef CONFIG_SYS_USE_FLASH
#else
#define CONFIG_SYS_USE_FLASH
#undef CONFIG_SYS_USE_NANDFLASH
#endif

#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_BASEDIR	"cpu9G20"
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_BASEDIR	"cpu9260"
#endif

#if defined(CONFIG_SYS_USE_FLASH)
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_SECT_SIZE		0x20000
#define	CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BOOTCOMMAND		"run flashboot"

#define MTDIDS_DEFAULT		"nor0=physmap-flash.0,nand0=atmel_nand"
#define MTDPARTS_DEFAULT		\
	"mtdparts=physmap-flash.0:"	\
		"256k(u-boot)ro,"	\
		"128k(u-boot-env)ro,"	\
		"1792k(kernel),"	\
		"-(rootfs);"		\
	"atmel_nand:-(nand)"

#define CONFIG_BOOTARGS "root=/dev/mtdblock3 rootfstype=jffs2 "

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"mtdids=" MTDIDS_DEFAULT "\0"				\
	"mtdparts=" MTDPARTS_DEFAULT "\0"			\
	"partition=nand0,0\0"					\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"	\
	"ramboot=tftpboot 0x22000000 $(basedir)/uImage;"	\
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
#elif defined(CONFIG_NANDBOOT)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_ENV_OFFSET_REDUND	0x80000
#define CONFIG_ENV_SECT_SIZE		0x20000
#define	CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BOOTCOMMAND		"run flashboot"

#define MTDIDS_DEFAULT		"nand0=atmel_nand"
#define MTDPARTS_DEFAULT		\
	"mtdparts=atmel_nand:"		\
		"128k(bootstrap)ro,"	\
		"256k(u-boot)ro,"	\
		"128k(u-boot-env)ro,"	\
		"128k(u-boot-env2)ro,"	\
		"2M(kernel),"	\
		"-(rootfs)"

#define CONFIG_BOOTARGS "root=ubi0:eukrea-cpu9260-rootfs "	\
	"ubi.mtd=5 rootfstype=ubifs at91sam9_wdt.heartbeat=60"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"mtdids=" MTDIDS_DEFAULT "\0"				\
	"mtdparts=" MTDPARTS_DEFAULT "\0"			\
	"partition=nand0,5\0"					\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"	\
	"ramboot=tftpboot 0x22000000 $(basedir)/uImage;"	\
		"run ramargs;bootm 22000000\0"			\
	"flashboot=run ramargs; nand read 0x22000000 0xA0000 "	\
		"0x200000; bootm 0x22000000\0"			\
	"basedir=" CONFIG_SYS_BASEDIR "\0"			\
	"u-boot=u-boot-eukrea-cpu9260.bin\0"			\
	"kernel=uImage-eukrea-cpu9260.bin\0"			\
	"rootfs=image-eukrea-cpu9260.ubi\0"			\
	"updtub=tftp ${loadaddr} $(basedir)/${u-boot}; "	\
		"nand erase 20000 40000; "			\
		"nand write ${loadaddr} 20000 40000\0"		\
	"updtui=tftp ${loadaddr} $(basedir)/${kernel}; "	\
		"nand erase a0000 200000; "			\
		"nand write ${loadaddr} a0000 200000\0"		\
	"updtrfs=tftp ${loadaddr} $(basedir)/${rootfs}; "	\
		"nand erase  2a0000 fd60000; "			\
		"nand write ${loadaddr} 2a0000 ${filesize}\0"
#endif

#define CONFIG_BAUDRATE			115200

#if defined(CONFIG_CPU9G20)
#define CONFIG_SYS_PROMPT		"CPU9G20=> "
#elif defined(CONFIG_CPU9260)
#define CONFIG_SYS_PROMPT		"CPU9260=> "
#endif
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		\
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SILENT_CONSOLE
#define CONFIG_NETCONSOLE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		\
		ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 0x1000)

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - \
				GENERATED_GBL_DATA_SIZE)

#endif
