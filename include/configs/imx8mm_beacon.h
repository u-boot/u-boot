/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Compass Electronics Group, LLC
 */

#ifndef __IMX8MM_BEACON_H
#define __IMX8MM_BEACON_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x930000
/* For RAW image gives a error info not panic */

#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"script=boot.scr\0" \
	"image=Image\0" \
	"console=ttymxc1,115200\0" \
	"fdt_addr=0x43000000\0"			\
	"boot_fit=try\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"initrd_addr=0x43800000\0"		\
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=1\0" \
	"finduuid=part uuid mmc ${mmcdev}:2 uuid\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate}" \
	" root=PARTUUID=${uuid} rootwait rw ${mtdparts} ${optargs}\0" \
	"loadbootscript=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr}" \
	" ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run finduuid; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"booti ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi; " \
	"netargs=setenv bootargs console=${console} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs;  " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${loadaddr} ${image}; " \
		"if test ${boot_fit} = yes || test ${boot_fit} = try; then " \
			"bootm ${loadaddr}; " \
		"else " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"booti ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"echo WARN: Cannot load the DT; " \
			"fi; " \
		"fi;\0"

/* Link Definitions */

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        0x200000

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE		0x80000000 /* 2GB DDR */

#endif
