/*
 * Copyright (C) 2013 Samsung Electronics
 * Sanghee Kim <sh0130.kim@samsung.com>
 * Piotr Wilczek <p.wilczek@samsung.com>
 *
 * Configuation settings for the SAMSUNG TRATS2 (EXYNOS4412) board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TRATS2_H
#define __CONFIG_TRATS2_H

#include <configs/exynos4-common.h>

#define CONFIG_TIZEN			/* TIZEN lib */

#define CONFIG_SYS_L2CACHE_OFF
#ifndef CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE	0x10502000
#endif

/* TRATS2 has 4 banks of DRAM */
#define CONFIG_NR_DRAM_BANKS		4
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM_1			CONFIG_SYS_SDRAM_BASE
#define SDRAM_BANK_SIZE			(256 << 20)	/* 256 MB */
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5E00000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x3E00000)

#define CONFIG_SYS_TEXT_BASE		0x43e00000

/* select serial console configuration */
#define CONFIG_SERIAL2
#define CONFIG_BAUDRATE			115200

/* Console configuration */
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_BOOTARGS			"Please use defined boot"
#define CONFIG_BOOTCOMMAND		"run autoboot"
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC2,115200n8\0"

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR \
					- GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MEM_TOP_HIDE	(1 << 20)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		CONFIG_MMC_DEFAULT_DEV
#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_OFFSET		((32 - 4) << 10) /* 32KiB - 4KiB */

#define CONFIG_ENV_OVERWRITE

#define CONFIG_ENV_VARS_UBOOT_CONFIG
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* Tizen - partitions definitions */
#define PARTS_CSA		"csa-mmc"
#define PARTS_BOOT		"boot"
#define PARTS_QBOOT		"qboot"
#define PARTS_CSC		"csc"
#define PARTS_ROOT		"platform"
#define PARTS_DATA		"data"
#define PARTS_UMS		"ums"

#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name="PARTS_CSA",start=5MiB,size=8MiB,uuid=${uuid_gpt_"PARTS_CSA"};" \
	"name="PARTS_BOOT",size=60MiB,uuid=${uuid_gpt_"PARTS_BOOT"};" \
	"name="PARTS_QBOOT",size=100MiB,uuid=${uuid_gpt_"PARTS_QBOOT"};" \
	"name="PARTS_CSC",size=150MiB,uuid=${uuid_gpt_"PARTS_CSC"};" \
	"name="PARTS_ROOT",size=1536MiB,uuid=${uuid_gpt_"PARTS_ROOT"};" \
	"name="PARTS_DATA",size=3000MiB,uuid=${uuid_gpt_"PARTS_DATA"};" \
	"name="PARTS_UMS",size=-,uuid=${uuid_gpt_"PARTS_UMS"}\0" \

#define CONFIG_DFU_ALT \
	"u-boot raw 0x80 0x800;" \
	"/uImage ext4 0 2;" \
	"/modem.bin ext4 0 2;" \
	"/exynos4412-trats2.dtb ext4 0 2;" \
	""PARTS_CSA" part 0 1;" \
	""PARTS_BOOT" part 0 2;" \
	""PARTS_QBOOT" part 0 3;" \
	""PARTS_CSC" part 0 4;" \
	""PARTS_ROOT" part 0 5;" \
	""PARTS_DATA" part 0 6;" \
	""PARTS_UMS" part 0 7;" \
	"params.bin raw 0x38 0x8;" \
	"/Image.itb ext4 0 2\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootk=" \
		"run loaduimage;" \
		"if run loaddtb; then " \
			"bootm 0x40007FC0 - ${fdtaddr};" \
		"fi;" \
		"bootm 0x40007FC0;\0" \
	"updatebackup=" \
		"mmc dev 0 2; mmc write 0x51000000 0 0x800;" \
		" mmc dev 0 0\0" \
	"updatebootb=" \
		"mmc read 0x51000000 0x80 0x800; run updatebackup\0" \
	"mmcboot=" \
		"setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcrootpart} " \
		"${lpj} rootwait ${console} ${meminfo} ${opts} ${lcdinfo}; " \
		"run bootk\0" \
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
	CONFIG_EXTRA_ENV_ITB \
	"fdtaddr=40800000\0" \

/* GPT */
#define CONFIG_RANDOM_UUID

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

/* Security subsystem - enable hw_rand() */
#define CONFIG_EXYNOS_ACE_SHA
#define CONFIG_LIB_HW_RAND

/* Common misc for Samsung */
#define CONFIG_MISC_COMMON

#define CONFIG_MISC_INIT_R

/* Download menu - Samsung common */
#define CONFIG_LCD_MENU
#define CONFIG_LCD_MENU_BOARD

/* Download menu - definitions for check keys */
#ifndef __ASSEMBLY__
#include <power/max77686_pmic.h>

#define KEY_PWR_PMIC_NAME		"MAX77686_PMIC"
#define KEY_PWR_STATUS_REG		MAX77686_REG_PMIC_STATUS1
#define KEY_PWR_STATUS_MASK		(1 << 0)
#define KEY_PWR_INTERRUPT_REG		MAX77686_REG_PMIC_INT1
#define KEY_PWR_INTERRUPT_MASK		(1 << 1)

#define KEY_VOL_UP_GPIO			EXYNOS4X12_GPIO_X22
#define KEY_VOL_DOWN_GPIO		EXYNOS4X12_GPIO_X33
#endif /* __ASSEMBLY__ */

/* LCD console */
#define LCD_BPP                 LCD_COLOR16
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
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE ((500 * 160 * 4) + 54)

#endif	/* __CONFIG_H */
