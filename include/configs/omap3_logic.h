/*
 * (C) Copyright 2011 Logic Product Development <www.logicpd.com>
 *	Peter Barada <peter.barada@logicpd.com>
 *
 * Configuration settings for the Logic OMAP35x/DM37x SOM LV/Torpedo
 * reference boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP			/* in a TI OMAP core */
#define CONFIG_OMAP3_LOGIC		/* working with Logic OMAP boards */
#define CONFIG_OMAP_GPIO
#define CONFIG_OMAP_COMMON
/* Common ARM Erratas */
#define CONFIG_ARM_ERRATA_454179
#define CONFIG_ARM_ERRATA_430973
#define CONFIG_ARM_ERRATA_621766

#define CONFIG_SYS_TEXT_BASE	0x80400000

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>	/* get chip and board defs */
#include <asm/arch/omap.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R		/* misc_init_r dumps the die id */

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_CMDLINE_EDITING			/* cmd line edit/history */
#define CONFIG_ZERO_BOOTDELAY_CHECK		/* check keypress w/no delay */

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
						/* Sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10))

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1
#define CONFIG_SERIAL1			1	/* UART1 on OMAP Logic boards */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

/* commands to include */
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_JFFS2	/* JFFS2 Support		*/
#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:512k(x-loader),"\
					"1920k(u-boot),128k(u-boot-env),"\
					"4m(kernel),-(fs)"

#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_NAND_LOCK_UNLOCK	/* nand (un)lock commands	*/
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

#define CONFIG_SYS_NO_FLASH

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_OMAP24_I2C_SPEED	100000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_SYS_I2C_OMAP34XX

/*
 * TWL4030
 */
#define CONFIG_TWL4030_POWER

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

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
							/* NAND devices */
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT	16
#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* size of jffs2 */
							/* partition */

/* Environment information */
#define CONFIG_BOOTDELAY		2

/*
 * PREBOOT assumes the 4.3" display is attached.  User can interrupt
 * and modify display variable to suit their needs.
 */
#define CONFIG_PREBOOT \
	"echo ======================NOTICE============================;"\
	"echo \"The u-boot environment is not set.\";"			\
	"echo \"If using a display a valid display varible for your panel\";" \
	"echo \"needs to be set.\";"					\
	"echo \"Valid display options are:\";"				\
	"echo \"  2 == LQ121S1DG31     TFT SVGA    (12.1)  Sharp\";"	\
	"echo \"  3 == LQ036Q1DA01     TFT QVGA    (3.6)   Sharp w/ASIC\";" \
	"echo \"  5 == LQ064D343       TFT VGA     (6.4)   Sharp\";"	\
	"echo \"  7 == LQ10D368        TFT VGA     (10.4)  Sharp\";"	\
	"echo \" 15 == LQ043T1DG01     TFT WQVGA   (4.3)   Sharp (DEFAULT)\";" \
	"echo \" vga[-dvi or -hdmi]    LCD VGA     640x480\";"          \
	"echo \" svga[-dvi or -hdmi]   LCD SVGA    800x600\";"          \
	"echo \" xga[-dvi or -hdmi]    LCD XGA     1024x768\";"         \
	"echo \" 720p[-dvi or -hdmi]   LCD 720P    1280x720\";"         \
	"echo \"Defaulting to 4.3 LCD panel (display=15).\";"		\
	"setenv display 15;"						\
	"setenv preboot;"						\
	"saveenv;"


#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x81000000\0" \
	"bootfile=uImage\0" \
	"mtdids=" MTDIDS_DEFAULT "\0"	\
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"mmcdev=0\0" \
	"autoboot=mmc dev ${mmcdev}; if mmc rescan; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"run defaultboot;" \
			"fi; " \
		"else run defaultboot; fi\0" \
	"defaultboot=run mmcramboot\0" \
	"consoledevice=ttyO0\0" \
	"display=15\0" \
	"setconsole=setenv console ${consoledevice},${baudrate}n8\0" \
	"dump_bootargs=echo 'Bootargs: '; echo $bootargs\0" \
	"rotation=0\0" \
	"vrfb_arg=if itest ${rotation} -ne 0; then " \
		"setenv bootargs ${bootargs} omapfb.vrfb=y " \
		"omapfb.rotate=${rotation}; " \
		"fi\0" \
	"otherbootargs=ignore_loglevel early_printk no_console_suspend\0" \
	"addmtdparts=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"common_bootargs=setenv bootargs ${bootargs} display=${display} " \
		"${otherbootargs};" \
		"run addmtdparts; " \
		"run vrfb_arg\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo 'Running bootscript from mmc ...'; " \
		"source ${loadaddr}\0" \
	"loaduimage=mmc rescan ${mmcdev}; " \
		"fatload mmc ${mmcdev} ${loadaddr} ${bootfile}\0" \
	"ramdisksize=64000\0" \
	"ramdiskaddr=0x82000000\0" \
	"ramdiskimage=rootfs.ext2.gz.uboot\0" \
	"ramargs=run setconsole; setenv bootargs console=${console} " \
		"root=/dev/ram rw ramdisk_size=${ramdisksize}\0" \
	"mmcramboot=echo 'Booting kernel from mmc w/ramdisk...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"run loaduimage; " \
		"fatload mmc ${mmcdev} ${ramdiskaddr} ${ramdiskimage}; "\
		"bootm ${loadaddr} ${ramdiskaddr}\0" \
	"ramboot=echo 'Booting kernel/ramdisk rootfs from tftp...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"tftpboot ${loadaddr} ${bootfile}; "\
		"tftpboot ${ramdiskaddr} ${ramdiskimage}; "\
		"bootm ${loadaddr} ${ramdiskaddr}\0"

#define CONFIG_BOOTCOMMAND \
	"run autoboot"

#define CONFIG_AUTO_COMPLETE
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"OMAP Logic # "
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0) /* default load */
								/* address */

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#if defined(CONFIG_CMD_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#elif defined(CONFIG_CMD_ONENAND)
#define CONFIG_SYS_FLASH_BASE		ONENAND_MAP
#endif

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

#if defined(CONFIG_CMD_NAND)
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#endif

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/*
 * SMSC922x Ethernet
 */
#if defined(CONFIG_CMD_NET)

#define CONFIG_SMC911X
#define CONFIG_SMC911X_16_BIT
#define CONFIG_SMC911X_BASE	0x08000000

#endif /* (CONFIG_CMD_NET) */

/*
 * BOOTP fields
 */

#define CONFIG_BOOTP_SUBNETMASK		0x00000001
#define CONFIG_BOOTP_GATEWAY		0x00000002
#define CONFIG_BOOTP_HOSTNAME		0x00000004
#define CONFIG_BOOTP_BOOTPATH		0x00000010

#endif /* __CONFIG_H */
