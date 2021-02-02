/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2018 NXP
 * Copyright 2019 Siemens AG
 */

#ifndef __IMX8X_CAPRICORN_H
#define __IMX8X_CAPRICORN_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#include "siemens-env-common.h"
#include "siemens-ccp-common.h"

/* SPL config */
#ifdef CONFIG_SPL_BUILD

#define CONFIG_SPL_MAX_SIZE		(124 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(1024 * 1024)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR		0x800

#define CONFIG_SPL_LDSCRIPT		"arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_STACK		0x013E000
#define CONFIG_SPL_BSS_START_ADDR	0x00128000
#define CONFIG_SPL_BSS_MAX_SIZE		0x1000	/* 4 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x00120000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x3000	/* 12 KB */
#define CONFIG_MALLOC_F_ADDR		0x00120000

#define CONFIG_SPL_RAW_IMAGE_ARM_TRUSTED_FIRMWARE
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#endif /* CONFIG_SPL_BUILD */

#define CONFIG_FACTORYSET

#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING		GENERATE_CCP_VERSION("01", "07")

#define CONFIG_REMAKE_ELF

/* ENET Config */
#define CONFIG_FEC_XCV_TYPE		RMII
#define FEC_QUIRK_ENET_MAC

/* ENET1 connects to base board and MUX with ESAI */
#define CONFIG_FEC_ENET_DEV		1
#define CONFIG_FEC_MXC_PHYADDR		0x0
#define CONFIG_ETHPRIME                "eth1"

/* I2C Configuration */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SYS_I2C_SPEED	400000
/* EEPROM */
#define  EEPROM_I2C_BUS		0 /* I2C0 */
#define  EEPROM_I2C_ADDR	0x50
/* PCA9552 */
#define  PCA9552_1_I2C_BUS	1 /* I2C1 */
#define  PCA9552_1_I2C_ADDR	0x60
#endif /* !CONFIG_SPL_BUILD */

/* AHAB */
#ifdef CONFIG_AHAB_BOOT
#define AHAB_ENV "sec_boot=yes\0"
#else
#define AHAB_ENV "sec_boot=no\0"
#endif

#define MFG_ENV_SETTINGS_DEFAULT \
	"mfgtool_args=setenv bootargs console=${console},${baudrate} " \
		"rdinit=/linuxrc " \
		"clk_ignore_unused "\
		"\0" \
	"kboot=booti\0"\
	"bootcmd_mfg=run mfgtool_args;" \
	"if iminfo ${initrd_addr}; then " \
	"if test ${tee} = yes; then " \
		"bootm ${tee_addr} ${initrd_addr} ${fdt_addr}; " \
	"else " \
		"booti ${loadaddr} ${initrd_addr} ${fdt_addr}; " \
	"fi; " \
	"else " \
	    "echo \"Run fastboot ...\"; fastboot 0; "  \
	"fi;\0"

/* Boot M4 */
#define M4_BOOT_ENV \
	"m4_0_image=m4_0.bin\0" \
	"loadm4image_0=fatload mmc ${mmcdev}:${mmcpart} " \
			"${loadaddr} ${m4_0_image}\0" \
	"m4boot_0=run loadm4image_0; dcache flush; bootaux ${loadaddr} 0\0" \

#define CONFIG_MFG_ENV_SETTINGS \
	MFG_ENV_SETTINGS_DEFAULT \
	"initrd_addr=0x83100000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"emmc_dev=0\0"

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_MFG_ENV_SETTINGS \
	M4_BOOT_ENV \
	AHAB_ENV \
	ENV_COMMON \
	"script=boot.scr\0" \
	"image=Image\0" \
	"panel=NULL\0" \
	"console=ttyLP2\0" \
	"fdt_addr=0x83000000\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"cntr_addr=0x88000000\0" \
	"cntr_file=os_cntr_signed.bin\0" \
	"initrd_addr=0x83800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"netdev=eth0\0" \
	"nfsopts=vers=3,udp,rsize=4096,wsize=4096,nolock rw\0" \
	"hostname=capricorn\0" \
	ENV_EMMC \
	ENV_NET

#define CONFIG_BOOTCOMMAND \
	"if usrbutton; then " \
		"run flash_self_test; " \
		"reset; " \
	"fi;" \
	"run flash_self;" \
	"reset;"

/* Default location for tftp and bootm */
#define CONFIG_LOADADDR			0x80280000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_INIT_SP_ADDR		0x80200000

/* On CCP board, USDHC1 is for eMMC */
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"  /* eMMC */
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		((CONFIG_ENV_SIZE + (32 * 1024)) * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x880000000
/* DDR3 board total DDR is 1 GB */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1 GB */
#define PHYS_SDRAM_2_SIZE		0x00000000	/* 0 GB */

/* Console buffer and boot args */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		8000000	/* 8MHz */

#define BOOTAUX_RESERVED_MEM_BASE	0x88000000
#define BOOTAUX_RESERVED_MEM_SIZE	SZ_128M /* Reserve from second 128MB */

#endif /* __IMX8X_CAPRICORN_H */
