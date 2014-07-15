/*
 * (C) Copyright 2009-2012
 * Jens Scharsig  <esw@bus-elekronik.de>
 * BuS Elektronik GmbH & Co. KG
 *
 * Configuation settings for the VL_MA2SC board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*--------------------------------------------------------------------------*/

#define CONFIG_ARM926EJS		/* This is an ARM926EJS Core	*/
#define CONFIG_AT91FAMILY
#define CONFIG_AT91SAM9263		/* It's an Atmel AT91SAM9263 SoC*/
#define CONFIG_VL_MA2SC			/* on an VL_MA2SC Board	*/
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_MISC_INIT_R

#include <asm/hardware.h>

#define MACH_TYPE_VL_MA2SC		2412
#define CONFIG_MACH_TYPE		MACH_TYPE_VL_MA2SC

#define CONFIG_SYS_DCACHE_OFF

#ifdef CONFIG_RAMLOAD
#define CONFIG_SYS_TEXT_BASE		0x21000000
#else
#define CONFIG_SYS_TEXT_BASE		0x00000000
#endif
#define CONFIG_SYS_LOAD_ADDR		0x21000000  /* default load address */

#define CONFIG_IDENT_STRING		" on MiS Activ 2"
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AT91_GPIO

#if !defined(CONFIG_SYS_USE_NANDFLASH) && !defined(CONFIG_RAMLOAD)
#define CONFIG_SYS_USE_NORFLASH
#define CONFIG_SYS_USE_BOOT_NORFLASH
#endif

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#ifndef CONFIG_SYS_USE_BOOT_NORFLASH
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/*
 * Hardware drivers
 */

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_WATCHDOG

#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS

/* LCD */
#define CONFIG_LCD
#define CONFIG_ATMEL_LCD
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SYS_BLACK_ON_WHITE
#define LCD_BPP				LCD_COLOR8
#define CONFIG_ATMEL_LCD_BGR555

#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_BOOTDELAY		3

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
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_LOADS

#define CONFIG_CMD_BMP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_MD5SUM
#define CONFIG_CMD_SHA1SUM
/*
#define CONFIG_CMD_SPI
*/
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB

#define	CONFIG_SYS_LONGHELP
#define CONFIG_MD5
#define	CONFIG_SHA1

/*----------------------------------------------------------------------------
 * Hardware confuguration
 *---------------------------------------------------------------------------*/

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00a00000	/* UHP_BASE */
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9263"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE
#define CONFIG_AT91C_PQFP_UHPBUG

/* I2C-Bus */

#define CONFIG_SYS_I2C_SPEED			50000
#define CONFIG_SYS_I2C_SLAVE			0		/* not used */

#ifndef CONFIG_HARD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT			/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	CONFIG_SYS_I2C_SPEED
#define CONFIG_SYS_I2C_SOFT_SLAVE	CONFIG_SYS_I2C_SLAVE

/* Software  I2C driver configuration */
#define I2C_DELAY	udelay(2500000/CONFIG_SYS_I2C_SPEED)

#define AT91_PIN_SDA	(1<<4)		/* AT91C_PIO_PB4 */
#define AT91_PIN_SCL	(1<<5)		/* AT91C_PIO_PB5 */

#define I2C_INIT	i2c_init_board();
#define I2C_ACTIVE	writel(AT91_PIN_SDA, &pio->piob.mddr);
#define I2C_TRISTATE	writel(AT91_PIN_SDA, &pio->piob.mder);
#define I2C_READ	((readl(&pio->piob.pdsr) & AT91_PIN_SDA) != 0)
#define I2C_SDA(bit)						\
	do {							\
		if (bit)					\
			writel(AT91_PIN_SDA, &pio->piob.sodr);	\
		else						\
			writel(AT91_PIN_SDA, &pio->piob.codr);	\
	} while (0);
#define I2C_SCL(bit)						\
	do {							\
		if (bit)					\
			writel(AT91_PIN_SCL, &pio->piob.sodr);	\
		else						\
			writel(AT91_PIN_SCL, &pio->piob.codr);	\
	} while (0);
#endif

/* I2C-RTC */

#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_DS1338
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#endif

/* EEPROM */

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50

/* define PDC[31:16] as DATA[31:16] */
#define CONFIG_SYS_PIOD_PDR_VAL1	0xFFFF0000
#define CONFIG_SYS_PIOD_PPUDR_VAL	0xFFFF0000

/* EBI0_CSA, CS1 SDRAM, CS3 NAND Flash, 3.3V memories */
#define CONFIG_SYS_MATRIX_EBI0CSA_VAL					\
	(AT91_MATRIX_CSA_DBPUC | AT91_MATRIX_CSA_VDDIOMSEL_3_3V |	\
	 AT91_MATRIX_CSA_EBI_CS1A)

/* user reset enable */
#define CONFIG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_MR_URSTEN |		\
		AT91_RSTC_MR_ERSTL(15))

/* Disable Watchdog */
#define CONFIG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_MR_WDIDLEHLT | AT91_WDT_MR_WDDBGHLT |	\
		 AT91_WDT_MR_WDV(0xFFF) |			\
		 AT91_WDT_MR_WDDIS |				\
		 AT91_WDT_MR_WDD(0xFFF))

/* clocks */

#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock */

#define MHZ180
#if defined(MHZ199)
/* 199,8994 MHZ */
#define MASTER_PLL_MUL		911
#define MASTER_PLL_DIV		56
#define MASTER_PLL_OUT		2
#elif defined(MHZ180)
/* 180 MHZ */
#define MASTER_PLL_MUL		1875
#define MASTER_PLL_DIV		128
#define MASTER_PLL_OUT		2
#elif defined(MHZTEST)
/* Test MHZ */
#define CONFIG_DISPLAY_CPUINFO
#define MASTER_PLL_MUL		8
#define MASTER_PLL_DIV		1
#define MASTER_PLL_OUT		2
#else
/* 176.9472 MHZ */
#define MASTER_PLL_MUL		72
#define MASTER_PLL_DIV		5
#define MASTER_PLL_OUT		2
#endif

#define CONFIG_SYS_MOR_VAL					\
	(AT91_PMC_MOR_MOSCEN | AT91_PMC_MOR_OSCOUNT(255))

#define CONFIG_SYS_PLLAR_VAL					\
	(AT91_PMC_PLLAR_29 |					\
	AT91_PMC_PLLXR_OUT(MASTER_PLL_OUT) |			\
	AT91_PMC_PLLXR_PLLCOUNT(63) |				\
	AT91_PMC_PLLXR_MUL(MASTER_PLL_MUL - 1) |		\
	AT91_PMC_PLLXR_DIV(MASTER_PLL_DIV))

/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR1_VAL		\
	(AT91_PMC_MCKR_CSS_SLOW | AT91_PMC_MCKR_PRES_1 |	\
	 AT91_PMC_MCKR_MDIV_2)

/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR2_VAL		\
	(AT91_PMC_MCKR_CSS_PLLA | AT91_PMC_MCKR_PRES_1 |	\
	AT91_PMC_MCKR_MDIV_2)

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x04000000  /* 64 megs */
#define CONFIG_SYS_INIT_SP_ADDR		0x00504000  /* use internal SRAM0 */

#define CONFIG_SYS_SDRC_MR_VAL1		0
#define CONFIG_SYS_SDRC_TR_VAL1		700
#define CONFIG_SYS_SDRC_CR_VAL						\
		(AT91_SDRAMC_NC_9 |					\
		 AT91_SDRAMC_NR_13 |					\
		 AT91_SDRAMC_NB_4 |					\
		 AT91_SDRAMC_CAS_3 |					\
		 AT91_SDRAMC_DBW_32 |					\
		 (2 <<  8) |		/* Write Recovery Delay */	\
		 (7 << 12) |		/* Row Cycle Delay */		\
		 (2 << 16) |		/* Row Precharge Delay */	\
		 (2 << 20) |		/* Row to Column Delay */	\
		 (5 << 24) |		/* Active to Precharge Delay */	\
		 (8 << 28))		/* Exit Self Refresh to Active Delay */

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

/* NOR flash */

#define CONFIG_FLASH_SHOW_PROGRESS	45
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define PHYS_FLASH_1			0x10000000
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x00060000)

/* setup SMC0, CS0 (NOR Flash) - 16-bit, 15 WS */
#define CONFIG_SYS_SMC0_SETUP0_VAL				\
	(AT91_SMC_SETUP_NWE(10) | AT91_SMC_SETUP_NCS_WR(10) |	\
	 AT91_SMC_SETUP_NRD(10) | AT91_SMC_SETUP_NCS_RD(10))
#define CONFIG_SYS_SMC0_PULSE0_VAL				\
	(AT91_SMC_PULSE_NWE(11) | AT91_SMC_PULSE_NCS_WR(11) |	\
	 AT91_SMC_PULSE_NRD(11) | AT91_SMC_PULSE_NCS_RD(11))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
	(AT91_SMC_CYCLE_NWE(22) | AT91_SMC_CYCLE_NRD(22))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
	(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |		\
	 AT91_SMC_MODE_DBW_16 |					\
	 AT91_SMC_MODE_TDF | AT91_SMC_MODE_TDF_CYCLE(6))

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8		1
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)	/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)	/* our CLE is AD22 */
#define CONFIG_SYS_NAND_ENABLE_PIN	GPIO_PIN_PD(15)
#define CONFIG_SYS_NAND_READY_PIN	GPIO_PIN_PB(0)
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_MULTI
#define CONFIG_NET_RETRY_COUNT		5
#define CONFIG_AT91_WANTS_COMMON_PHY

#define CONFIG_OVERWRITE_ETHADDR_ONCE

#define CONFIG_SYS_LOAD_ADDR		0x21000000  /* default load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x21e00000

/* Address and size of Primary Environment Sector */
#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SIZE			0x20000
#else
#define CONFIG_ENV_SIZE			0x2000
#endif

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{312500, 230400, 115200, 19200, \
						38400, 57600, 9600 }

#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS	32		/* max number of command args */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		\
	ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 0x1000)

#ifndef CONFIG_RAMLOAD
#define CONFIG_BOOTCOMMAND		"run nfsboot"
#endif
#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_BOOT_RETRY_MIN		15

#define CONFIG_NFSBOOTCOMMAND						\
		"dhcp $(copy_addr) $(kernelname);"			\
		"run bootargsdefaults;"					\
		"set bootargs $(bootargs) boot=nfs "			\
		";echo $(bootargs)"					\
	";bootm"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"ubootaddr=10000000\0"						\
	"splashimage=10080000\0"					\
	"kerneladdr=100A0000\0"						\
	"kernelsize=00800000\0"						\
	"minifsaddr=108A0000\0"						\
	"minifssize=00060000\0"						\
	"rootfsaddr=10900000\0"						\
	"copy_addr=20200000\0"						\
	"rootfssize=01700000\0"						\
	"kernelname=uImage_vl_ma2sc\0"					\
	"bootargsdefaults=set bootargs "				\
		"console=ttyS0,115200 "					\
		"video=atmel_lcdfb "					\
		"mem=62M "						\
		"panic=10 "						\
		"boardrevison=\\\"${revision}\\\" "			\
		"uboot=\\\"${ver}\\\" "					\
		"\0"							\
	"update_all=run update_kernel;run update_root;"			\
		"run update_splash; run update_uboot\0"			\
	"update_kernel=protect off $(kerneladdr) +$(kernelsize);"	\
		"dhcp $(copy_addr) $(kernelname);"			\
		"erase $(kerneladdr) +$(kernelsize);"			\
		"cp.b $(fileaddr) $(kerneladdr) $(filesize);"		\
		"protect on $(kerneladdr) +$(kernelsize)"		\
		"\0"							\
	"update_root=protect off $(rootfsaddr) +$(rootfssize);"		\
		"dhcp $(copy_addr) vl_ma2sc.root;"			\
		"erase $(rootfsaddr) +$(rootfssize);"			\
		"cp.b $(fileaddr) $(rootfsaddr) $(filesize);"		\
		"\0"							\
	"update_splash=protect off $(splashimage) +20000;"		\
		"dhcp $(copy_addr) splash_vl_ma2sc.bmp;"		\
		"erase $(splashimage) +20000;"				\
		"cp.b $(fileaddr) 10080000 $(filesize);"		\
		"protect on $(splashimage) +20000\0"			\
	"update_uboot=protect off 10000000 1005FFFF;"			\
		"dhcp $(copy_addr) u-boot_vl_ma2sc;"			\
		"erase 10000000 1005FFFF;"				\
		"cp.b $(fileaddr) $(ubootaddr) $(filesize);"		\
		"protect on 10000000 1005FFFF;reset\0"			\
	"emergency=run bootargsdefaults;"				\
		"set bootargs $(bootargs) root=initramfs boot=emergency " \
		";bootm $(kerneladdr)\0"				\
	"netemergency=run bootargsdefaults;"				\
		"dhcp $(copy_addr) $(kernelname);"			\
		"set bootargs $(bootargs) root=initramfs boot=emergency " \
		";bootm $(copy_addr)\0"					\
	"norboot=run bootargsdefaults;"					\
		"set bootargs $(bootargs) root=initramfs boot=local quiet " \
		";bootm $(kerneladdr)\0"				\
	"nandboot=run bootargsdefaults;"				\
		"set bootargs $(bootargs) root=initramfs boot=nand "	\
		";bootm $(kerneladdr)\0"				\
	"setnorboot=set bootcmd 'run norboot'; set bootdelay 1;save\0"	\
	"clearenv=protect off 10060000 1007FFFF;"			\
		"erase 10060000 1007FFFF;reset\0"			\
	" "

#endif
