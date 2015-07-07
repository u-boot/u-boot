/*
 * Copyright (C) 2013 Gumstix, Inc. - http://www.gumstix.com/
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_PEPPER_H
#define __CONFIG_PEPPER_H

#define CONFIG_MMC
#include <configs/ti_am335x_common.h>

#undef CONFIG_BOARD_LATE_INIT
#undef CONFIG_SPL_OS_BOOT

/* Clock defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"pepper# "

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50

/* Mach type */
#define MACH_TYPE_PEPPER		4207	/* Until the next sync */
#define CONFIG_MACH_TYPE		MACH_TYPE_PEPPER

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_IS_NOWHERE
/* Display cpuinfo */
#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"fdtfile=am335x-pepper.dtb\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"mmcload=load mmc ${mmcdev}:2 ${loadaddr} ${bootdir}/${bootfile}; " \
		"load mmc ${mmcdev}:2 ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"loaduimage=fatload mmc ${mmcdev}:1 ${loadaddr} uImage\0" \
	"uimageboot=echo Booting from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"ubiboot=echo Booting from nand (ubifs) ...; " \
		"run ubiargs; run ubiload; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run mmcload; then " \
			"run mmcboot;" \
		"fi;" \
		"if run loaduimage; then " \
			"run uimageboot;" \
		"fi;" \
	"fi;" \

/* Serial console configuration */
#define CONFIG_CONS_INDEX		1 /* UART0 */
#define CONFIG_SERIAL1			1
#define CONFIG_SYS_NS16550_COM1		0x44e09000

/* Ethernet support */
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_ADDR			0
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9021
#define CONFIG_PHY_RESET_DELAY 1000

/* SPL */
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/am33xx/u-boot-spl.lds"

#endif /* __CONFIG_PEPPER_H */
