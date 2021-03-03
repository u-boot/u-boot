/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Toradex
 */

#ifndef __APALIS_IMX8X_H
#define __APALIS_IMX8X_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include <linux/stringify.h>

#define CONFIG_REMAKE_ELF

#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define USDHC1_BASE_ADDR		0x5b010000
#define USDHC2_BASE_ADDR		0x5b020000
#define CONFIG_SUPPORT_EMMC_BOOT	/* eMMC specific */

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_TFTP_TSIZE

#define CONFIG_IPADDR			192.168.10.2
#define CONFIG_NETMASK			255.255.255.0
#define CONFIG_SERVERIP			192.168.10.1

#define MEM_LAYOUT_ENV_SETTINGS \
	"kernel_addr_r=0x80280000\0" \
	"fdt_addr_r=0x83100000\0" \
	"ramdisk_addr_r=0x8a000000\0" \
	"scriptaddr=0x83200000\0"

#ifdef CONFIG_AHAB_BOOT
#define AHAB_ENV "sec_boot=yes\0"
#else
#define AHAB_ENV "sec_boot=no\0"
#endif

/* Boot M4 */
#define M4_BOOT_ENV \
	"m4_0_image=m4_0.bin\0" \
	"loadm4image_0=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${m4_0_image}\0" \
	"m4boot_0=run loadm4image_0; dcache flush; bootaux ${loadaddr} 0\0" \

#define MFG_NAND_PARTITION ""

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#undef BOOTENV_RUN_NET_USB_START
#define BOOTENV_RUN_NET_USB_START ""

#define CONFIG_MFG_ENV_SETTINGS \
	"mfgtool_args=setenv bootargs ${consoleargs} " \
		"rdinit=/linuxrc g_mass_storage.stall=0 " \
		"g_mass_storage.removable=1 g_mass_storage.idVendor=0x066F " \
		"g_mass_storage.idProduct=0x37FF " \
		"g_mass_storage.iSerialNumber=\"\" " MFG_NAND_PARTITION \
		"${vidargs} clk_ignore_unused\0" \
	"initrd_addr=0x83800000\0" \
	"bootcmd_mfg=run mfgtool_args;booti ${loadaddr} ${initrd_addr} " \
		"${fdt_addr};\0" \

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	AHAB_ENV \
	BOOTENV \
	CONFIG_MFG_ENV_SETTINGS \
	M4_BOOT_ENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"boot_file=Image\0" \
	"consoleargs=console=ttyLP3,${baudrate} earlycon\0" \
	"fdt_file=imx8qxp-apalis-eval.dtb\0" \
	"fdtfile=imx8qxp-apalis-eval.dtb\0" \
	"finduuid=part uuid mmc ${mmcdev}:2 uuid\0" \
	"image=Image\0" \
	"initrd_addr=0x83800000\0" \
	"mmcargs=setenv bootargs ${consoleargs} " \
		"root=PARTUUID=${uuid} rootwait " \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"netargs=setenv bootargs ${consoleargs} " \
		"root=/dev/nfs ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp " \
		"${vidargs}\0" \
	"nfsboot=run netargs; dhcp ${loadaddr} ${image}; tftp ${fdt_addr} " \
		"apalis-imx8x/${fdt_file}; booti ${loadaddr} - " \
		"${fdt_addr}\0" \
	"panel=NULL\0" \
	"script=boot.scr\0" \
	"update_uboot=askenv confirm Did you load u-boot-dtb.imx (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 0 1; mmc write ${loadaddr} 0x0 " \
		"${blkcnt}; fi\0" \
	"vidargs=video=imxdpufb5:off video=imxdpufb6:off video=imxdpufb7:off\0"

/* Link Definitions */
#define CONFIG_LOADADDR			0x89000000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_SP_ADDR		0x80200000

/* Environment in eMMC, before config block at the end of 1st "boot sector" */

#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* On Apalis iMX8X USDHC1 is eMMC, USDHC2 is 4-bit SD */
#define CONFIG_SYS_FSL_USDHC_NUM	2

#define CONFIG_SYS_BOOTM_LEN		SZ_64M /* Increase max gunzip size */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		((CONFIG_ENV_SIZE + (32 * 1024)) * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x880000000
#define PHYS_SDRAM_1_SIZE		SZ_2G		/* 2 GB */
#define PHYS_SDRAM_2_SIZE		0x00000000	/* 0 GB */

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_CBSIZE		SZ_2K
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		8000000	/* 8MHz */

/* Networking */
#define CONFIG_FEC_ENET_DEV 0
#define IMX_FEC_BASE			0x5b040000
#define CONFIG_FEC_MXC_PHYADDR          0x4
#define CONFIG_ETHPRIME                 "eth0"
#define CONFIG_FEC_XCV_TYPE		RGMII
#define PHY_ANEG_TIMEOUT 20000

#endif /* __APALIS_IMX8X_H */
