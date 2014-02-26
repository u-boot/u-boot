/*
 * Copyright (C) 2011 Samsung Electronics
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * Configuation settings for the SAMSUNG TRATS (EXYNOS4210) board.
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
#define CONFIG_EXYNOS4210	/* which is in a EXYNOS4210 */
#define CONFIG_TRATS		/* working with TRATS */
#define CONFIG_TIZEN		/* TIZEN lib */

#include <asm/arch/cpu.h>	/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_L2CACHE_OFF
#ifndef CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE	0x10502000
#endif

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x63300000

/* input clock of PLL: TRATS has 24MHz input clock at EXYNOS4210 */
#define CONFIG_SYS_CLK_FREQ_C210	24000000
#define CONFIG_SYS_CLK_FREQ		CONFIG_SYS_CLK_FREQ_C210

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

/* MACH_TYPE_TRATS macro will be removed once added to mach-types */
#define MACH_TYPE_TRATS			3928
#define CONFIG_MACH_TYPE		MACH_TYPE_TRATS

#include <linux/sizes.h>
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (80 * SZ_1M))

/* select serial console configuration */
#define CONFIG_SERIAL2			/* use SERIAL 2 */
#define CONFIG_BAUDRATE			115200

/* MMC */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_S5P_SDHCI
#define CONFIG_SDHCI
#define CONFIG_MMC_SDMA

/* PWM */
#define CONFIG_PWM

/* It should define before config_cmd_default.h */
#define CONFIG_SYS_NO_FLASH

/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MISC
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_XIMG
#undef CONFIG_CMD_CACHE
#undef CONFIG_CMD_ONENAND
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_MMC
#define CONFIG_CMD_DFU
#define CONFIG_CMD_GPT
#define CONFIG_CMD_SETEXPR

/* FAT */
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE

/* USB Composite download gadget - g_dnl */
#define CONFIG_USBDOWNLOAD_GADGET

/* TIZEN THOR downloader support */
#define CONFIG_CMD_THOR_DOWNLOAD
#define CONFIG_THOR_FUNCTION

#define CONFIG_SYS_DFU_DATA_BUF_SIZE SZ_32M
#define DFU_DEFAULT_POLL_TIMEOUT 300
#define CONFIG_DFU_FUNCTION
#define CONFIG_DFU_MMC

/* USB Samsung's IDs */
#define CONFIG_G_DNL_VENDOR_NUM 0x04E8
#define CONFIG_G_DNL_PRODUCT_NUM 0x6601
#define CONFIG_G_DNL_THOR_VENDOR_NUM CONFIG_G_DNL_VENDOR_NUM
#define CONFIG_G_DNL_THOR_PRODUCT_NUM 0x685D
#define CONFIG_G_DNL_MANUFACTURER "Samsung"

#define CONFIG_BOOTDELAY		1
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTARGS			"Please use defined boot"
#define CONFIG_BOOTCOMMAND		"run mmcboot"

#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC2,115200n8\0"
#define CONFIG_BOOTBLOCK		"10"
#define CONFIG_ENV_COMMON_BOOT		"${console} ${meminfo}"

/* Tizen - partitions definitions */
#define PARTS_CSA		"csa-mmc"
#define PARTS_BOOTLOADER	"u-boot"
#define PARTS_BOOT		"boot"
#define PARTS_ROOT		"platform"
#define PARTS_DATA		"data"
#define PARTS_CSC		"csc"
#define PARTS_UMS		"ums"

#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name="PARTS_CSA",size=8MiB,uuid=${uuid_gpt_"PARTS_CSA"};" \
	"name="PARTS_BOOTLOADER",size=60MiB," \
		"uuid=${uuid_gpt_"PARTS_BOOTLOADER"};" \
	"name="PARTS_BOOT",size=100MiB,uuid=${uuid_gpt_"PARTS_BOOT"};" \
	"name="PARTS_ROOT",size=1GiB,uuid=${uuid_gpt_"PARTS_ROOT"};" \
	"name="PARTS_DATA",size=3GiB,uuid=${uuid_gpt_"PARTS_DATA"};" \
	"name="PARTS_CSC",size=150MiB,uuid=${uuid_gpt_"PARTS_CSC"};" \
	"name="PARTS_UMS",size=-,uuid=${uuid_gpt_"PARTS_UMS"}\0" \

#define CONFIG_DFU_ALT \
	"u-boot mmc 80 400;" \
	"uImage ext4 0 2;" \
	"exynos4210-trats.dtb ext4 0 2;" \
	""PARTS_BOOT" part 0 2;" \
	""PARTS_ROOT" part 0 5;" \
	""PARTS_DATA" part 0 6;" \
	""PARTS_UMS" part 0 7;" \
	"params.bin mmc 0x38 0x8\0"

#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_ENV_VARS_UBOOT_CONFIG
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootk=" \
		"run loaduimage;" \
		"if run loaddtb; then " \
			"bootm 0x40007FC0 - ${fdtaddr};" \
		"fi;" \
		"bootm 0x40007FC0;\0" \
	"updatemmc=" \
		"mmc boot 0 1 1 1; mmc write 0 0x42008000 0 0x200;" \
		"mmc boot 0 1 1 0\0" \
	"updatebackup=" \
		"mmc boot 0 1 1 2; mmc write 0 0x42100000 0 0x200;" \
		"mmc boot 0 1 1 0\0" \
	"updatebootb=" \
		"mmc read 0 0x42100000 0x80 0x200; run updatebackup\0" \
	"lpj=lpj=3981312\0" \
	"nfsboot=" \
		"setenv bootargs root=/dev/nfs rw " \
		"nfsroot=${nfsroot},nolock,tcp " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:" \
		"${netmask}:generic:usb0:off " CONFIG_ENV_COMMON_BOOT \
		"; run bootk\0" \
	"ramfsboot=" \
		"setenv bootargs root=/dev/ram0 rw rootfstype=ext2 " \
		"${console} ${meminfo} " \
		"initrd=0x43000000,8M ramdisk=8192\0" \
	"mmcboot=" \
		"setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcrootpart} " \
		"${lpj} rootwait ${console} ${meminfo} ${opts} ${lcdinfo}; " \
		"run bootk\0" \
	"bootchart=setenv opts init=/sbin/bootchartd; run bootcmd\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"mmcoops=mmc read 0 0x40000000 0x40 8; md 0x40000000 0x400\0" \
	"verify=n\0" \
	"rootfstype=ext4\0" \
	"console=" CONFIG_DEFAULT_CONSOLE \
	"meminfo=crashkernel=32M@0x50000000\0" \
	"nfsroot=/nfsroot/arm\0" \
	"bootblock=" CONFIG_BOOTBLOCK "\0" \
	"loaduimage=ext4load mmc ${mmcdev}:${mmcbootpart} 0x40007FC0 uImage\0" \
	"loaddtb=ext4load mmc ${mmcdev}:${mmcbootpart} ${fdtaddr} " \
		"${fdtfile}\0" \
	"mmcdev=0\0" \
	"mmcbootpart=2\0" \
	"mmcrootpart=5\0" \
	"opts=always_resume=1\0" \
	"partitions=" PARTS_DEFAULT \
	"dfu_alt_info=" CONFIG_DFU_ALT \
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


/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"TRATS # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4800000)

/* TRATS has 4 banks of DRAM */
#define CONFIG_NR_DRAM_BANKS	4
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_2		(CONFIG_SYS_SDRAM_BASE + SDRAM_BANK_SIZE)
#define PHYS_SDRAM_2_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_3		(CONFIG_SYS_SDRAM_BASE + (2 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_3_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_4		(CONFIG_SYS_SDRAM_BASE + (3 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_4_SIZE	SDRAM_BANK_SIZE

#define CONFIG_SYS_MEM_TOP_HIDE		(1 << 20)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_OFFSET		((32 - 4) << 10) /* 32KiB - 4KiB */

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

/* EXT4 */
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
/* Falcon mode definitions */
#define CONFIG_CMD_SPL
#define CONFIG_SYS_SPL_ARGS_ADDR        PHYS_SDRAM_1 + 0x100

/* GPT */
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUIDS

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_CACHELINE_SIZE       32

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_S3C24X0
#define CONFIG_SYS_I2C_S3C24X0_SPEED	100000
#define CONFIG_SYS_I2C_S3C24X0_SLAVE	0xFE
#define CONFIG_MAX_I2C_NUM		8
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7F
#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define CONFIG_SYS_I2C_INIT_BOARD

#include <asm/arch/gpio.h>

/* I2C FG */
#define CONFIG_SOFT_I2C_GPIO_SCL exynos4_gpio_get(2, y4, 1)
#define CONFIG_SOFT_I2C_GPIO_SDA exynos4_gpio_get(2, y4, 0)

#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_MAX8997

#define CONFIG_POWER_FG
#define CONFIG_POWER_FG_MAX17042
#define CONFIG_POWER_MUIC
#define CONFIG_POWER_MUIC_MAX8997
#define CONFIG_POWER_BATTERY
#define CONFIG_POWER_BATTERY_TRATS
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_S3C_UDC_OTG
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2
#define CONFIG_USB_CABLE_CHECK

/* Common misc for Samsung */
#define CONFIG_MISC_COMMON

#define CONFIG_MISC_INIT_R

/* Download menu - Samsung common */
#define CONFIG_LCD_MENU
#define CONFIG_LCD_MENU_BOARD

/* Download menu - definitions for check keys */
#ifndef __ASSEMBLY__
#include <power/max8997_pmic.h>

#define KEY_PWR_PMIC_NAME		"MAX8997_PMIC"
#define KEY_PWR_STATUS_REG		MAX8997_REG_STATUS1
#define KEY_PWR_STATUS_MASK		(1 << 0)
#define KEY_PWR_INTERRUPT_REG		MAX8997_REG_INT1
#define KEY_PWR_INTERRUPT_MASK		(1 << 0)

#define KEY_VOL_UP_GPIO			exynos4_gpio_get(2, x2, 0)
#define KEY_VOL_DOWN_GPIO		exynos4_gpio_get(2, x2, 1)
#endif /* __ASSEMBLY__ */

/* LCD console */
#define LCD_BPP			LCD_COLOR16
#define CONFIG_SYS_WHITE_ON_BLACK

/* LCD */
#define CONFIG_EXYNOS_FB
#define CONFIG_LCD
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_FB_ADDR		0x52504000
#define CONFIG_S6E8AX0
#define CONFIG_EXYNOS_MIPI_DSIM
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE  ((500 * 160 * 4) + 54)

#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USB_GADGET_MASS_STORAGE

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT    1

#endif	/* __CONFIG_H */
