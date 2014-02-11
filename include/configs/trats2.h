/*
 * Copyright (C) 2013 Samsung Electronics
 * Sanghee Kim <sh0130.kim@samsung.com>
 * Piotr Wilczek <p.wilczek@samsung.com>
 *
 * Configuation settings for the SAMSUNG TRATS2 (EXYNOS4412) board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SAMSUNG		/* in a SAMSUNG core */
#define CONFIG_S5P		/* which is in a S5P Family */
#define CONFIG_EXYNOS4		/* which is in a EXYNOS4XXX */
#define CONFIG_TIZEN		/* TIZEN lib */

#include <asm/arch/cpu.h>		/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_L2CACHE_OFF
#ifndef CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE	0x10502000
#endif

#define CONFIG_NR_DRAM_BANKS	4
#define PHYS_SDRAM_1		0x40000000	/* LDDDR2 DMC 0 */
#define PHYS_SDRAM_1_SIZE	(256 << 20)	/* 256 MB in CS 0 */
#define PHYS_SDRAM_2		0x50000000	/* LPDDR2 DMC 1 */
#define PHYS_SDRAM_2_SIZE	(256 << 20)	/* 256 MB in CS 0 */
#define PHYS_SDRAM_3		0x60000000	/* LPDDR2 DMC 1 */
#define PHYS_SDRAM_3_SIZE	(256 << 20)	/* 256 MB in CS 0 */
#define PHYS_SDRAM_4		0x70000000	/* LPDDR2 DMC 1 */
#define PHYS_SDRAM_4_SIZE	(256 << 20)	/* 256 MB in CS 0 */
#define PHYS_SDRAM_END		0x80000000

#define CONFIG_SYS_MEM_TOP_HIDE		(1 << 20)	/* ram console */

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_TEXT_BASE		0x78100000

#define CONFIG_SYS_CLK_FREQ		24000000

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG

/* MACH_TYPE_TRATS2 */
#define MACH_TYPE_TRATS2		3765
#define CONFIG_MACH_TYPE		MACH_TYPE_TRATS2

#define CONFIG_DISPLAY_CPUINFO

#include <asm/sizes.h>
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (80 * SZ_1M))

/* select serial console configuration */
#define CONFIG_SERIAL2

#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser	*/
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define CONFIG_CMDLINE_EDITING

#define CONFIG_BAUDRATE			115200

/* It should define before config_cmd_default.h */
#define CONFIG_SYS_NO_FLASH

/***********************************************************
 * Command definition
 ***********************************************************/
#include <config_cmd_default.h>

#undef CONFIG_CMD_ECHO
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_NAND
#undef CONFIG_CMD_MISC
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SOURCE
#undef CONFIG_CMD_XIMG
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MMC
#define CONFIG_CMD_DFU
#define CONFIG_CMD_GPT
#define CONFIG_CMD_PMIC

#define CONFIG_BOOTDELAY	3
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE

/* EXT4 */
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE

/* USB Composite download gadget - g_dnl */
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_SYS_DFU_DATA_BUF_SIZE SZ_32M
#define DFU_DEFAULT_POLL_TIMEOUT 300
#define CONFIG_DFU_FUNCTION
#define CONFIG_DFU_MMC

/* TIZEN THOR downloader support */
#define CONFIG_CMD_THOR_DOWNLOAD
#define CONFIG_THOR_FUNCTION

/* USB Samsung's IDs */
#define CONFIG_G_DNL_VENDOR_NUM 0x04E8
#define CONFIG_G_DNL_PRODUCT_NUM 0x6601
#define CONFIG_G_DNL_THOR_VENDOR_NUM CONFIG_G_DNL_VENDOR_NUM
#define CONFIG_G_DNL_THOR_PRODUCT_NUM 0x685D
#define CONFIG_G_DNL_MANUFACTURER "Samsung"

/* To use the TFTPBOOT over USB, Please enable the CONFIG_CMD_NET */
#undef CONFIG_CMD_NET

/* MMC */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_S5P_SDHCI
#define CONFIG_SDHCI
#define CONFIG_MMC_SDMA
#define CONFIG_MMC_DEFAULT_DEV	0

/* PWM */
#define CONFIG_PWM

#define CONFIG_BOOTARGS		"Please use defined boot"
#define CONFIG_BOOTCOMMAND	"run mmcboot"
#define CONFIG_DEFAULT_CONSOLE	"console=ttySAC2,115200n8\0"

#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* Tizen - partitions definitions */
#define PARTS_CSA		"csa"
#define PARTS_BOOT		"boot"
#define PARTS_MODEM		"modem"
#define PARTS_CSC		"csc"
#define PARTS_ROOT		"platform"
#define PARTS_DATA		"data"
#define PARTS_UMS		"ums"

#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name="PARTS_CSA",start=5MiB,size=8MiB,uuid=${uuid_gpt_"PARTS_CSA"};" \
	"name="PARTS_BOOT",size=64MiB,uuid=${uuid_gpt_"PARTS_BOOT"};" \
	"name="PARTS_MODEM",size=100MiB,uuid=${uuid_gpt_"PARTS_MODEM"};" \
	"name="PARTS_CSC",size=150MiB,uuid=${uuid_gpt_"PARTS_CSC"};" \
	"name="PARTS_ROOT",size=1536MiB,uuid=${uuid_gpt_"PARTS_ROOT"};" \
	"name="PARTS_DATA",size=512MiB,uuid=${uuid_gpt_"PARTS_DATA"};" \
	"name="PARTS_UMS",size=-,uuid=${uuid_gpt_"PARTS_UMS"}\0" \

#define CONFIG_DFU_ALT \
	"u-boot mmc 80 800;" \
	"uImage ext4 0 2;" \
	"exynos4412-trats2.dtb ext4 0 2;" \
	""PARTS_BOOT" part 0 2;" \
	""PARTS_ROOT" part 0 5;" \
	""PARTS_DATA" part 0 6;" \
	""PARTS_UMS" part 0 7\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootk=" \
		"run loaddtb; run loaduimage; bootm 0x40007FC0 - ${fdtaddr}\0" \
	"updatemmc=" \
		"mmc boot 0 1 1 1; mmc write 0x42008000 0 0x200;" \
		"mmc boot 0 1 1 0\0" \
	"updatebackup=" \
		"mmc boot 0 1 1 2; mmc write 0x42100000 0 0x200;" \
		" mmc boot 0 1 1 0\0" \
	"updatebootb=" \
		"mmc read 0x51000000 0x80 0x200; run updatebackup\0" \
	"updateuboot=" \
		"mmc write 0x50000000 0x80 0x400\0" \
	"mmcboot=" \
		"setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcrootpart} " \
		"${lpj} rootwait ${console} ${meminfo} ${opts} ${lcdinfo}; " \
		"run loaddtb; run loaduimage; bootm 0x40007FC0 - ${fdtaddr}\0" \
	"bootchart=set opts init=/sbin/bootchartd; run bootcmd\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"verify=n\0" \
	"rootfstype=ext4\0" \
	"console=" CONFIG_DEFAULT_CONSOLE \
	"kernelname=uImage\0" \
	"loaduimage=ext4load mmc ${mmcdev}:${mmcbootpart} 0x40007FC0 " \
		"${kernelname}\0" \
	"loaddtb=ext4load mmc ${mmcdev}:${mmcbootpart} ${fdtaddr} " \
		"${fdtfile}\0" \
	"mmcdev=" __stringify(CONFIG_MMC_DEFAULT_DEV) "\0" \
	"mmcbootpart=2\0" \
	"mmcrootpart=5\0" \
	"opts=always_resume=1\0" \
	"partitions=" PARTS_DEFAULT \
	"dfu_alt_info=" CONFIG_DFU_ALT \
	"uartpath=ap\0" \
	"usbpath=ap\0" \
	"consoleon=set console console=ttySAC2,115200n8; save; reset\0" \
	"consoleoff=set console console=ram; save; reset\0" \
	"spladdr=0x40000100\0" \
	"splsize=0x200\0" \
	"splfile=falcon.bin\0" \
	"spl_export=" \
		   "setexpr spl_imgsize ${splsize} + 8 ;" \
		   "setenv spl_imgsize 0x${spl_imgsize};" \
		   "setexpr spl_imgaddr ${spladdr} - 8 ;" \
		   "setexpr spl_addr_tmp ${spladdr} - 4 ;" \
		   "mw.b ${spl_imgaddr} 0x00 ${spl_imgsize};run loaduimage;" \
		   "setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcrootpart} " \
		   "${lpj} rootwait ${console} ${meminfo} ${opts} ${lcdinfo};" \
		   "spl export atags 0x40007FC0;" \
		   "crc32 ${spladdr} ${splsize} ${spl_imgaddr};" \
		   "mw.l ${spl_addr_tmp} ${splsize};" \
		   "ext4write mmc ${mmcdev}:${mmcbootpart}" \
		   " /${splfile} ${spl_imgaddr} ${spl_imgsize};" \
		   "setenv spl_imgsize;" \
		   "setenv spl_imgaddr;" \
		   "setenv spl_addr_tmp;\0" \
	"fdtaddr=40800000\0" \
	"fdtfile=exynos4412-trats2.dtb\0"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_PROMPT	"Trats2 # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	384		/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	32		/* max number of command args */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4800000)

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_LOAD_ADDR \
					- GENERATED_GBL_DATA_SIZE)

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_SYS_MONITOR_BASE		0x00000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		CONFIG_MMC_DEFAULT_DEV
#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_OFFSET		((32 - 4) << 10) /* 32KiB - 4KiB */
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUIDS

#define CONFIG_MISC_INIT_R
#define CONFIG_BOARD_EARLY_INIT_F

/* I2C */
#include <asm/arch/gpio.h>

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_S3C24X0
#define CONFIG_SYS_I2C_S3C24X0_SPEED	100000
#define CONFIG_SYS_I2C_S3C24X0_SLAVE	0
#define CONFIG_MAX_I2C_NUM		8
#define CONFIG_SYS_I2C_SOFT
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x00
#define I2C_SOFT_DECLARATIONS2
#define CONFIG_SYS_I2C_SOFT_SPEED_2     50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_2     0x00
#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define CONFIG_SYS_I2C_INIT_BOARD

#ifndef __ASSEMBLY__
int get_soft_i2c_scl_pin(void);
int get_soft_i2c_sda_pin(void);
#endif
#define CONFIG_SOFT_I2C_GPIO_SCL	get_soft_i2c_scl_pin()
#define CONFIG_SOFT_I2C_GPIO_SDA	get_soft_i2c_sda_pin()

/* POWER */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_MAX77686
#define CONFIG_POWER_PMIC_MAX77693
#define CONFIG_POWER_MUIC_MAX77693
#define CONFIG_POWER_FG_MAX77693
#define CONFIG_POWER_BATTERY_TRATS2
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_S3C_UDC_OTG
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2
#define CONFIG_USB_CABLE_CHECK

/* LCD */
#define CONFIG_EXYNOS_FB
#define CONFIG_LCD
#define CONFIG_CMD_BMP
#define CONFIG_BMP_32BPP
#define CONFIG_FB_ADDR		0x52504000
#define CONFIG_S6E8AX0
#define CONFIG_EXYNOS_MIPI_DSIM
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE ((500 * 250 * 4) + (1 << 12))

#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USB_GADGET_MASS_STORAGE

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT    1

#endif	/* __CONFIG_H */
