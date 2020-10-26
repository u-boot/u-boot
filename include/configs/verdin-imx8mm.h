/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Toradex
 */

#ifndef __VERDIN_IMX8MM_H
#define __VERDIN_IMX8MM_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_STACK		0x920000
#define CONFIG_SPL_BSS_START_ADDR	0x910000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K	/* 8 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K	/* 512 KB */

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x930000
/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE
#endif

#define MEM_LAYOUT_ENV_SETTINGS \
	"fdt_addr_r=0x44000000\0" \
	"kernel_addr_r=0x42000000\0" \
	"ramdisk_addr_r=0x46400000\0" \
	"scriptaddr=0x46000000\0"

#define CONFIG_LOADADDR		0x40480000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* Enable Distro Boot */
#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#undef CONFIG_ISO_PARTITION
#else
#define BOOTENV
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"bootcmd_mfg=fastboot 0\0" \
	"console=ttymxc0\0" \
	"fdt_addr=0x43000000\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"initrd_addr=0x43800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"kernel_image=Image\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp" \
		"\0" \
	"nfsboot=run netargs; dhcp ${loadaddr} ${kernel_image}; " \
		"tftp ${fdt_addr} verdin/${fdtfile}; " \
		"booti ${loadaddr} - ${fdt_addr}\0" \
	"setup=setenv setupargs console=${console},${baudrate} " \
		"console=tty1 consoleblank=0 earlycon\0" \
	"update_uboot=askenv confirm Did you load flash.bin (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 0 1; mmc write ${loadaddr} 0x2 " \
		"${blkcnt}; fi\0"

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        SZ_2M
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#if defined(CONFIG_ENV_IS_IN_MMC)
/* Environment in eMMC, before config block at the end of 1st "boot sector" */
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		SZ_32M
#define CONFIG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_2G /* 2GB DDR */

/* UART */
#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		SZ_2K
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
/* USDHC */
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1
#define CONFIG_SYS_I2C_SPEED		100000

/* ENET */
#define CONFIG_ETHPRIME                 "FEC"
#define CONFIG_FEC_XCV_TYPE             RGMII
#define CONFIG_FEC_MXC_PHYADDR          7
#define FEC_QUIRK_ENET_MAC
#define IMX_FEC_BASE			0x30BE0000

#endif /*_VERDIN_IMX8MM_H */

