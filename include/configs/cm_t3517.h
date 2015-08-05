/*
 * (C) Copyright 2013 CompuLab, Ltd.
 * Author: Igor Grinberg <grinberg@compulab.co.il>
 *
 * Configuration settings for the CompuLab CM-T3517 board
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP	/* in a TI OMAP core */
#define CONFIG_CM_T3517	/* working with CM-T3517 */
#define CONFIG_OMAP_COMMON
#define CONFIG_SYS_GENERIC_BOARD
/* Common ARM Erratas */
#define CONFIG_ARM_ERRATA_454179
#define CONFIG_ARM_ERRATA_430973
#define CONFIG_ARM_ERRATA_621766

#define CONFIG_SYS_TEXT_BASE	0x80008000

/*
 * This is needed for the DMA stuff.
 * Although the default iss 64, we still define it
 * to be on the safe side once the default is changed.
 */
#define CONFIG_SYS_CACHELINE_SIZE	64

#define CONFIG_EMIF4	/* The chip has EMIF4 controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_OF_LIBFDT
/*
 * The early kernel mapping on ARM currently only maps from the base of DRAM
 * to the end of the kernel image.  The kernel is loaded at DRAM base + 0x8000.
 * The early kernel pagetable uses DRAM base + 0x4000 to DRAM base + 0x8000,
 * so that leaves DRAM base to DRAM base + 0x4000 available.
 */
#define CONFIG_SYS_BOOTMAPSZ	        0x4000

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SERIAL_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE		(16 << 10)	/* 16 KiB */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + (128 << 10))

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		48000000	/* 48MHz (APLL96/2) */

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3	/* UART3 */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

#define CONFIG_OMAP_GPIO

#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

/* USB */
#define CONFIG_USB_MUSB_AM35X

#ifndef CONFIG_USB_MUSB_AM35X
#define CONFIG_USB_OMAP3
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_OMAP
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO 146
#define CONFIG_OMAP_EHCI_PHY2_RESET_GPIO 147
#else /* !CONFIG_USB_MUSB_AM35X */
#define CONFIG_MUSB_HOST
#define CONFIG_MUSB_PIO_ONLY
#endif /* CONFIG_USB_MUSB_AM35X */

#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* commands to include */
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT		"nand0=nand"
#define MTDPARTS_DEFAULT	"mtdparts=nand:512k(x-loader),"\
				"1920k(u-boot),256k(u-boot-env),"\
				"4m(kernel),-(fs)"

#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_GPIO


#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_I2C
#define CONFIG_SYS_OMAP24_I2C_SPEED	400000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_SYS_I2C_OMAP34XX
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_BUS	0
#define CONFIG_I2C_MULTI_BUS

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_QUIET_TEST
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */

/* Environment information */
#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"baudrate=115200\0" \
	"console=ttyO2,115200n8\0" \
	"mpurate=auto\0" \
	"vram=12M\0" \
	"dvimode=1024x768MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw rootwait\0" \
	"mmcrootfstype=ext4\0" \
	"nandroot=/dev/mtdblock4 rw\0" \
	"nandrootfstype=ubifs\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 2a0000 400000; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_CMD_BOOTZ
#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run nandboot; " \
			"fi; " \
		"fi; " \
	"else run nandboot; fi"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_TIMESTAMP
#define CONFIG_SYS_AUTOLOAD		"no"
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"CM-T3517 # "
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		32	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0 + 0x02000000)

/*
 * AM3517 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	/* CM-T3517 DRAM is only on CS0 */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define CONFIG_SYS_CS0_SIZE		(256 << 20)

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */
/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_ENV_IS_IN_NAND
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			SMNAND_ENV_OFFSET

#if defined(CONFIG_CMD_NET)
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_MII
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE	(0x2C000000 + (16 << 20))
#endif /* CONFIG_CMD_NET */

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR +	\
					 CONFIG_SYS_INIT_RAM_SIZE -	\
					 GENERATED_GBL_DATA_SIZE)

/* Status LED */
#define CONFIG_STATUS_LED		/* Status LED enabled */
#define CONFIG_BOARD_SPECIFIC_LED
#define CONFIG_GPIO_LED
#define GREEN_LED_GPIO			186 /* CM-T3517 Green LED is GPIO186 */
#define GREEN_LED_DEV			0
#define STATUS_LED_BIT			GREEN_LED_GPIO
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BOOT			GREEN_LED_DEV

/* GPIO banks */
#ifdef CONFIG_STATUS_LED
#define CONFIG_OMAP3_GPIO_6	/* GPIO186 is in GPIO bank 6  */
#endif

/* Display Configuration */
#define CONFIG_OMAP3_GPIO_2
#define CONFIG_OMAP3_GPIO_5
#define CONFIG_VIDEO_OMAP3
#define LCD_BPP		LCD_COLOR16

#define CONFIG_LCD
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASHIMAGE_GUARD
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_SCF0403_LCD

#define CONFIG_OMAP3_SPI

#endif /* __CONFIG_H */
