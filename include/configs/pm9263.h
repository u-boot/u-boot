/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 *
 * Configuation settings for the RONETIX PM9263 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define MASTER_PLL_DIV		6
#define MASTER_PLL_MUL		65
#define MAIN_PLL_DIV		2	/* 2 or 4 */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */

#define CONFIG_SYS_AT91_CPU_NAME	"AT91SAM9263"
#define CONFIG_PM9263		1	/* on a Ronetix PM9263 Board	*/
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SYS_TEXT_BASE	0

#define MACH_TYPE_PM9263	1475
#define CONFIG_MACH_TYPE	MACH_TYPE_PM9263

/* clocks */
#define CONFIG_SYS_MOR_VAL						\
		(AT91_PMC_MOR_MOSCEN |					\
		 (255 << 8))		/* Main Oscillator Start-up Time */
#define CONFIG_SYS_PLLAR_VAL						\
		(AT91_PMC_PLLAR_29 | /* Bit 29 must be 1 when prog */ \
		 AT91_PMC_PLLXR_OUT(3) |				\
		 AT91_PMC_PLLXR_PLLCOUNT(0x3f) |	/* PLL Counter */\
		 (2 << 28) |		/* PLL Clock Frequency Range */	\
		 ((MASTER_PLL_MUL - 1) << 16) | (MASTER_PLL_DIV))

#if (MAIN_PLL_DIV == 2)
/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR1_VAL		\
		(AT91_PMC_MCKR_CSS_SLOW |	\
		 AT91_PMC_MCKR_PRES_1 |	\
		 AT91_PMC_MCKR_MDIV_2)
/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR2_VAL		\
		(AT91_PMC_MCKR_CSS_PLLA |	\
		 AT91_PMC_MCKR_PRES_1 |	\
		 AT91_PMC_MCKR_MDIV_2)
#else
/* PCK/4 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR1_VAL			\
		(AT91_PMC_MCKR_CSS_SLOW |		\
		 AT91_PMC_MCKR_PRES_1 |		\
		 AT91_PMC_MCKR_MDIV_4)
/* PCK/4 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR2_VAL			\
		(AT91_PMC_MCKR_CSS_PLLA |		\
		 AT91_PMC_MCKR_PRES_1 |		\
		 AT91_PMC_MCKR_MDIV_4)
#endif
/* define PDC[31:16] as DATA[31:16] */
#define CONFIG_SYS_PIOD_PDR_VAL1	0xFFFF0000
/* no pull-up for D[31:16] */
#define CONFIG_SYS_PIOD_PPUDR_VAL	0xFFFF0000
/* EBI0_CSA, CS1 SDRAM, CS3 NAND Flash, 3.3V memories */
#define CONFIG_SYS_MATRIX_EBI0CSA_VAL					\
	(AT91_MATRIX_CSA_DBPUC | AT91_MATRIX_CSA_VDDIOMSEL_3_3V |	\
	 AT91_MATRIX_CSA_EBI_CS1A)

/* SDRAM */
/* SDRAMC_MR Mode register */
#define CONFIG_SYS_SDRC_MR_VAL1		0
/* SDRAMC_TR - Refresh Timer register */
#define CONFIG_SYS_SDRC_TR_VAL1		0x3AA
/* SDRAMC_CR - Configuration register*/
#define CONFIG_SYS_SDRC_CR_VAL							\
		(AT91_SDRAMC_NC_9 |						\
		 AT91_SDRAMC_NR_13 |						\
		 AT91_SDRAMC_NB_4 |						\
		 AT91_SDRAMC_CAS_2 |						\
		 AT91_SDRAMC_DBW_32 |						\
		 (2 <<  8) |	/* tWR -  Write Recovery Delay */		\
		 (7 << 12) |	/* tRC -  Row Cycle Delay */			\
		 (2 << 16) |	/* tRP -  Row Precharge Delay */		\
		 (2 << 20) |	/* tRCD - Row to Column Delay */		\
		 (5 << 24) |	/* tRAS - Active to Precharge Delay */		\
		 (8 << 28))	/* tXSR - Exit Self Refresh to Active Delay */

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

/* setup SMC0, CS0 (NOR Flash) - 16-bit, 15 WS */
#define CONFIG_SYS_SMC0_SETUP0_VAL					\
		(AT91_SMC_SETUP_NWE(10) | AT91_SMC_SETUP_NCS_WR(10) |	\
		 AT91_SMC_SETUP_NRD(10) | AT91_SMC_SETUP_NCS_RD(10))
#define CONFIG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_PULSE_NWE(11) | AT91_SMC_PULSE_NCS_WR(11) |	\
		 AT91_SMC_PULSE_NRD(11) | AT91_SMC_PULSE_NCS_RD(11))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_CYCLE_NWE(22) | AT91_SMC_CYCLE_NRD(22))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |	\
		 AT91_SMC_MODE_DBW_16 |				\
		 AT91_SMC_MODE_TDF |				\
		 AT91_SMC_MODE_TDF_CYCLE(6))

/* user reset enable */
#define CONFIG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_CR_PROCRST |		\
		AT91_RSTC_MR_ERSTL(1) |	\
		AT91_RSTC_MR_ERSTL(2))

/* Disable Watchdog */
#define CONFIG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_MR_WDIDLEHLT | AT91_WDT_MR_WDDBGHLT |	\
		 AT91_WDT_MR_WDV(0xfff) |					\
		 AT91_WDT_MR_WDDIS |				\
		 AT91_WDT_MR_WDD(0xfff))

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#undef CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_USER_LOWLEVEL_INIT	1
#define CONFIG_BOARD_EARLY_INIT_F

/*
 * Hardware drivers
 */
#define CONFIG_AT91_GPIO	1
#define CONFIG_ATMEL_USART	1
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define	CONFIG_USART_ID			ATMEL_ID_SYS

/* LCD */
#define CONFIG_LCD			1
#define LCD_BPP				LCD_COLOR8
#define CONFIG_LCD_LOGO			1
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO			1
#define CONFIG_LCD_INFO_BELOW_LOGO	1
#define CONFIG_SYS_WHITE_ON_BLACK	1
#define CONFIG_ATMEL_LCD		1
#define CONFIG_ATMEL_LCD_BGR555		1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1

#define CONFIG_LCD_IN_PSRAM		1

/* LED */
#define CONFIG_AT91_LED
#define CONFIG_RED_LED		GPIO_PIN_PB(7) /* this is the power led */
#define CONFIG_GREEN_LED	GPIO_PIN_PB(8) /* this is the user1 led */

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

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_PING		1
#define CONFIG_CMD_DHCP		1
#define CONFIG_CMD_NAND		1
#define CONFIG_CMD_USB		1

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM		0x20000000
#define PHYS_SDRAM_SIZE		0x04000000	/* 64 megs */

/* DataFlash */
#define CONFIG_ATMEL_DATAFLASH_SPI
#define CONFIG_HAS_DATAFLASH			1
#define CONFIG_SYS_MAX_DATAFLASH_BANKS		1
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* CS0 */
#define AT91_SPI_CLK				15000000
#define DATAFLASH_TCSS				(0x1a << 16)
#define DATAFLASH_TCHS				(0x1 << 24)

/* NOR flash, if populated */
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define PHYS_FLASH_1			0x10000000
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_MAX_FLASH_BANKS	1

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8		1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	GPIO_PIN_PD(15)
#define CONFIG_SYS_NAND_READY_PIN	GPIO_PIN_PB(30)

#endif

#define CONFIG_CMD_JFFS2		1
#define CONFIG_JFFS2_CMDLINE		1
#define CONFIG_JFFS2_NAND		1
#define CONFIG_JFFS2_DEV		"nand0" /* NAND device jffs2 lives on */
#define CONFIG_JFFS2_PART_OFFSET	0	/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_SIZE		(256 * 1024 * 1024) /* partition size*/

/* PSRAM */
#define	PHYS_PSRAM			0x70000000
#define	PHYS_PSRAM_SIZE			0x00400000	/* 4MB */
/* Slave EBI1, PSRAM connected */
#define CONFIG_PSRAM_SCFG		(AT91_MATRIX_SCFG_ARBT_FIXED_PRIORITY	| \
					 AT91_MATRIX_SCFG_FIXED_DEFMSTR(5)	| \
					 AT91_MATRIX_SCFG_DEFMSTR_TYPE_FIXED	| \
					 AT91_MATRIX_SCFG_SLOT_CYCLE(255))

/* Ethernet */
#define CONFIG_MACB			1
#define CONFIG_RMII			1
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R		1

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW			1
#define CONFIG_DOS_PARTITION			1
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00a00000	/* AT91SAM9263_UHP_BASE */
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9263"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE			1

#define CONFIG_SYS_LOAD_ADDR			0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			0x23e00000

#define CONFIG_SYS_USE_FLASH	1
#undef CONFIG_SYS_USE_DATAFLASH
#undef CONFIG_SYS_USE_NANDFLASH

#ifdef CONFIG_SYS_USE_DATAFLASH

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_DATAFLASH
#define CFG_MONITOR_BASE	(CFG_DATAFLASH_LOGIC_ADDR_CS0 + 0x8400)
#define CONFIG_ENV_OFFSET	0x4200
#define CONFIG_ENV_ADDR		(CFG_DATAFLASH_LOGIC_ADDR_CS0 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x4200
#define CONFIG_BOOTCOMMAND	"cp.b 0xC0042000 0x22000000 0x210000; bootm"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 " \
				"root=/dev/mtdblock0 " \
				"mtdparts=atmel_nand:-(root) "\
				"rw rootfstype=jffs2"

#elif defined(CONFIG_SYS_USE_NANDFLASH) /* CFG_USE_NANDFLASH */

/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_ENV_OFFSET_REDUND	0x80000
#define CONFIG_ENV_SIZE		0x20000		/* 1 sector = 128 kB */
#define CONFIG_BOOTCOMMAND	"nand read 0x22000000 0xA0000 0x200000; bootm"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 "		\
				"root=/dev/mtdblock5 "		\
				"mtdparts=atmel_nand:"		\
					"128k(bootstrap)ro,"	\
					"256k(uboot)ro,"	\
					"128k(env1)ro,"		\
					"128k(env2)ro,"		\
					"2M(linux),"		\
					"-(root) "		\
				"rw rootfstype=jffs2"

#elif defined(CONFIG_SYS_USE_FLASH) /* CFG_USE_FLASH */

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET	0x40000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define	CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_OVERWRITE	1

/* JFFS Partition offset set */
#define CONFIG_SYS_JFFS2_FIRST_BANK	0
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

/* 512k reserved for u-boot */
#define CONFIG_SYS_JFFS2_FIRST_SECTOR	11

#define CONFIG_BOOTCOMMAND		"run flashboot"
#define CONFIG_ROOTPATH			"/ronetix/rootfs"
#define CONFIG_AUTOBOOT_PROMPT		"autoboot in %d seconds\n", bootdelay

#define CONFIG_CON_ROT			"fbcon=rotate:3 "
#define CONFIG_BOOTARGS			"root=/dev/mtdblock4 rootfstype=jffs2 "\
					CONFIG_CON_ROT

#define MTDIDS_DEFAULT			"nor0=physmap-flash.0,nand0=nand"
#define MTDPARTS_DEFAULT		\
	"mtdparts=physmap-flash.0:"	\
		"256k(u-boot)ro,"	\
		"64k(u-boot-env)ro,"	\
		"1408k(kernel),"	\
		"-(rootfs);"		\
	"nand:-(nand)"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"mtdids=" MTDIDS_DEFAULT "\0"				\
	"mtdparts=" MTDPARTS_DEFAULT "\0"			\
	"partition=nand0,0\0"					\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "		\
		CONFIG_CON_ROT					\
		"nfsroot=$(serverip):$(rootpath) $(mtdparts)\0"	\
	"addip=setenv bootargs $(bootargs) "			\
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"\
		":$(hostname):eth0:off\0"			\
	"ramboot=tftpboot 0x22000000 vmImage;"			\
		"run ramargs;run addip;bootm 22000000\0"	\
	"nfsboot=tftpboot 0x22000000 vmImage;"			\
		"run nfsargs;run addip;bootm 22000000\0"	\
	"flashboot=run ramargs;run addip;bootm 0x10050000\0"	\
	""

#else
#error "Undefined memory device"
#endif

#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_PROMPT		"u-boot-pm9263> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		\
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 0x1000)

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
				GENERATED_GBL_DATA_SIZE)

#endif
