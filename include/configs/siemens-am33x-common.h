/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * siemens am33x common board options
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/include/configs/am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_SIEMENS_AM33X_COMMON_H
#define __CONFIG_SIEMENS_AM33X_COMMON_H

#include <asm/arch/omap.h>

/* commands to include */

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Console I/O Buffer Size */

/*
 * memtest works on 8 MB in DRAM after skipping 32MB from
 * start addr of ram disk
 */

 /* Physical Memory Map */
#define PHYS_DRAM_1			0x80000000	/* DRAM Bank #1 */

#define CFG_SYS_SDRAM_BASE		PHYS_DRAM_1
 /* Platform/Board specific defs */
#define CFG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */

/* NS16550 Configuration */
#define CFG_SYS_NS16550_CLK		(48000000)
#define CFG_SYS_NS16550_COM1		0x44e09000
#define CFG_SYS_NS16550_COM4		0x481a6000

/* I2C Configuration */

/* Defines for SPL */

#define CFG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	14

#define	CFG_SYS_NAND_U_BOOT_START	CONFIG_TEXT_BASE

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */

/*
 * Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */

/* USB Device Firmware Update support */
#define DFU_MANIFEST_POLL_TIMEOUT	25000

/*
 * Default to using SPI for environment, etc.  We have multiple copies
 * of SPL as the ROM will check these locations.
 * 0x0 - 0x20000 : First copy of SPL
 * 0x20000 - 0x40000 : Second copy of SPL
 * 0x40000 - 0x60000 : Third copy of SPL
 * 0x60000 - 0x80000 : Fourth copy of SPL
 * 0x80000 - 0xDF000 : U-Boot
 * 0xDF000 - 0xE0000 : U-Boot Environment
 * 0xE0000 - 0x442000 : Linux Kernel
 * 0x442000 - 0x800000 : Userland
 */

/* NAND support */
#ifdef CONFIG_MTD_RAW_NAND
/* UBI Support */

/* Commen environment */
#define COMMON_ENV_DFU_ARGS	"dfu_args=run bootargs_defaults;" \
				"setenv bootargs ${bootargs};" \
				"mtdparts default;" \
				"draco_led 1;" \
				"dfu 0 nand 0;" \
				"draco_led 0;\0" \

#define COMMON_ENV_NAND_BOOT \
		"nand_boot=echo Booting from nand; " \
		"if test ${upgrade_available} -eq 1; then " \
			"if test ${bootcount} -gt ${bootlimit}; " \
				"then " \
				"setenv upgrade_available 0;" \
				"setenv ${partitionset_active} true;" \
				"if test -n ${A}; then " \
					"setenv partitionset_active B; " \
					"env delete A; " \
				"fi;" \
				"if test -n ${B}; then " \
					"setenv partitionset_active A; " \
					"env delete B; " \
				"fi;" \
				"saveenv; " \
			"fi;" \
		"fi;" \
		"echo set ${partitionset_active}...;" \
		"run nand_args; "

#define COMMON_ENV_NAND_CMDS	"flash_self=run nand_boot\0" \
				"flash_self_test=setenv testargs test; " \
					"run nand_boot\0" \
				"dfu_start=echo Preparing for dfu mode ...; " \
				"run dfu_args; \0"

#define COMMON_ENV_SETTINGS \
	"verify=no \0" \
	"project_dir=targetdir\0" \
	"upgrade_available=0\0" \
	"partitionset_active=A\0" \
	"loadaddr=0x82000000\0" \
	"kloadaddr=0x81000000\0" \
	"script_addr=0x81900000\0" \
	"console=console=ttyMTD,mtdoops console=ttyO0,115200n8 panic=5\0" \
	"nfsopts=nolock rw\0" \
	"ip_method=none\0" \
	"bootenv=uEnv.txt\0" \
	"bootargs_defaults=setenv bootargs " \
		"console=${console} " \
		"${testargs} " \
		"${optargs}\0" \
	"siemens_help=echo; "\
		"echo Type 'run flash_self' to use kernel and root " \
		"filesystem on memory; echo Type 'run flash_self_test' to " \
		"use kernel and root filesystem on memory, boot in test " \
		"mode; echo Not ready yet: 'run flash_nfs' to use kernel " \
		"from memory and root filesystem over NFS; echo Type " \
		"'run net_nfs' to get Kernel over TFTP and mount root " \
		"filesystem over NFS; " \
		"echo Set partitionset_active variable to 'A' " \
		"or 'B' to select kernel and rootfs partition; " \
		"echo" \
		"\0"

/*
 * Variant 1 partition layout
 * chip-size = 256MiB
 *|         name |        size |           address area |
 *-------------------------------------------------------
 *|          spl | 128.000 KiB | 0x       0..0x   1ffff |
 *|  spl.backup1 | 128.000 KiB | 0x   20000..0x   3ffff |
 *|  spl.backup2 | 128.000 KiB | 0x   40000..0x   5ffff |
 *|  spl.backup3 | 128.000 KiB | 0x   60000..0x   7ffff |
 *|       u-boot |   1.875 MiB | 0x   80000..0x  25ffff |
 *|    uboot.env | 128.000 KiB | 0x  260000..0x  27ffff |
 *|     kernel_a |   5.000 MiB | 0x  280000..0x  77ffff |
 *|     kernel_b |   5.000 MiB | 0x  780000..0x  c7ffff |
 *|      mtdoops |   8.000 MiB | 0x  c80000..0x 147ffff |
 *|       rootfs | 235.500 MiB | 0x 1480000..0x fffffff |
 *-------------------------------------------------------

					"mtdparts=omap2-nand.0:" \
					"128k(spl),"		\
					"128k(spl.backup1),"	\
					"128k(spl.backup2),"	\
					"128k(spl.backup3),"	\
					"1920k(u-boot),"	\
					"128k(uboot.env),"	\
					"5120k(kernel_a),"	\
					"5120k(kernel_b),"	\
					"8192k(mtdoops),"	\
					"-(rootfs)"
 */

#define DFU_ALT_INFO_NAND_V1 \
	"spl part 0 1;" \
	"spl.backup1 part 0 2;" \
	"spl.backup2 part 0 3;" \
	"spl.backup3 part 0 4;" \
	"u-boot part 0 5;" \
	"u-boot.env part 0 6;" \
	"kernel_a part 0 7;" \
	"kernel_b part 0 8;" \
	"rootfs partubi 0 10"

#define CFG_ENV_SETTINGS_NAND_V1 \
	"nand_active_ubi_vol=rootfs_a\0" \
	"nand_active_ubi_vol_A=rootfs_a\0" \
	"nand_active_ubi_vol_B=rootfs_b\0" \
	"nand_root_fs_type=ubifs rootwait\0" \
	"nand_src_addr=0x280000\0" \
	"nand_src_addr_A=0x280000\0" \
	"nand_src_addr_B=0x780000\0" \
	"nand_args=run bootargs_defaults;" \
		"mtdparts default;" \
		"setenv ${partitionset_active} true;" \
		"if test -n ${A}; then " \
			"setenv nand_active_ubi_vol ${nand_active_ubi_vol_A};" \
			"setenv nand_src_addr ${nand_src_addr_A};" \
		"fi;" \
		"if test -n ${B}; then " \
			"setenv nand_active_ubi_vol ${nand_active_ubi_vol_B};" \
			"setenv nand_src_addr ${nand_src_addr_B};" \
		"fi;" \
		"setenv nand_root ubi0:${nand_active_ubi_vol} rw " \
		"ubi.mtd=9,${ubi_off};" \
		"setenv bootargs ${bootargs} " \
		"root=${nand_root} noinitrd ${mtdparts} " \
		"rootfstype=${nand_root_fs_type} ip=${ip_method} " \
		"console=ttyMTD,mtdoops console=ttyO0,115200n8 mtdoops.mtddev" \
		"=mtdoops\0" \
	COMMON_ENV_DFU_ARGS \
		"dfu_alt_info=" DFU_ALT_INFO_NAND_V1 "\0" \
	COMMON_ENV_NAND_BOOT \
		"nand read.i ${kloadaddr} ${nand_src_addr} " \
		"${nand_img_size}; bootm ${kloadaddr}\0" \
	COMMON_ENV_NAND_CMDS

#define CFG_ENV_SETTINGS_V1 \
		COMMON_ENV_SETTINGS \
	"net_args=run bootargs_defaults;" \
		"mtdparts default;" \
		"setenv bootfile ${project_dir}/kernel/uImage;" \
		"setenv rootpath /home/projects/${project_dir}/rootfs;" \
		"setenv bootargs ${bootargs} " \
		"root=/dev/nfs ${mtdparts} " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} " \
		"ip=${ipaddr}:${serverip}:" \
		"${gatewayip}:${netmask}:${hostname}:eth0:off\0" \
	"net_nfs=echo Booting from network ...; " \
		"run net_args; " \
		"tftpboot ${kloadaddr} ${serverip}:${bootfile}; " \
		"bootm ${kloadaddr}\0"

/*
 * Variant 2 partition layout (default)
 * chip-size = 256MiB or 512 MiB
 *|         name |        size |           address area |
 *-------------------------------------------------------
 *|          spl | 128.000 KiB | 0x       0..0x   1ffff |
 *|  spl.backup1 | 128.000 KiB | 0x   20000..0x   3ffff |
 *|  spl.backup2 | 128.000 KiB | 0x   40000..0x   5ffff |
 *|  spl.backup3 | 128.000 KiB | 0x   60000..0x   7ffff |
 *|       u-boot |   1.875 MiB | 0x   80000..0x  25ffff |
 *|   uboot.env0 | 512.000 KiB | 0x  260000..0x  2Dffff |
 *|   uboot.env1 | 512.000 KiB | 0x  2E0000..0x  35ffff |
 *|      mtdoops | 512.000 KiB | 0x  360000..0x  3dffff |
 *| (256) rootfs | 252.125 MiB | 0x  3E0000..0x fffffff |
 *| (512) rootfs | 508.125 MiB | 0x  3E0000..0x1fffffff |
 *-------------------------------------------------------
 */

#define DFU_ALT_INFO_NAND_V2 \
	"spl part 0 1;" \
	"spl.backup1 part 0 2;" \
	"spl.backup2 part 0 3;" \
	"spl.backup3 part 0 4;" \
	"u-boot part 0 5;" \
	"u-boot.env0 part 0 6;" \
	"u-boot.env1 part 0 7;" \
	"rootfs partubi 0 9" \

#define CFG_ENV_SETTINGS_NAND_V2 \
	"nand_active_ubi_vol=rootfs_a\0" \
	"rootfs_name=rootfs\0" \
	"kernel_name=uImage\0"\
	"nand_root_fs_type=ubifs rootwait\0" \
	"nand_args=run bootargs_defaults;" \
		"mtdparts default;" \
		"setenv ${partitionset_active} true;" \
		"if test -n ${A}; then " \
			"setenv nand_active_ubi_vol ${rootfs_name}_a;" \
		"fi;" \
		"if test -n ${B}; then " \
			"setenv nand_active_ubi_vol ${rootfs_name}_b;" \
		"fi;" \
		"setenv nand_root ubi0:${nand_active_ubi_vol} rw " \
		"ubi.mtd=rootfs,2048;" \
		"setenv bootargs ${bootargs} " \
		"root=${nand_root} noinitrd ${mtdparts} " \
		"rootfstype=${nand_root_fs_type} ip=${ip_method} " \
		"console=ttyMTD,mtdoops console=ttyO0,115200n8 mtdoops.mtddev" \
		"=mtdoops\0" \
	COMMON_ENV_DFU_ARGS \
		"dfu_alt_info=" DFU_ALT_INFO_NAND_V2 "\0" \
	COMMON_ENV_NAND_BOOT \
		"ubi part rootfs ${ubi_off};" \
		"ubifsmount ubi0:${nand_active_ubi_vol};" \
		"ubifsload ${kloadaddr} boot/${kernel_name};" \
		"ubifsload ${loadaddr} boot/${dtb_name}.dtb;" \
		"bootm ${kloadaddr} - ${loadaddr}\0" \
	"nand_boot_backup=ubifsload ${loadaddr} boot/am335x-draco.dtb;" \
		"bootm ${kloadaddr} - ${loadaddr}\0" \
	COMMON_ENV_NAND_CMDS

#define CFG_ENV_SETTINGS_V2 \
		COMMON_ENV_SETTINGS \
	"net_args=run bootargs_defaults;" \
		"mtdparts default;" \
		"setenv bootfile ${project_dir}/kernel/uImage;" \
		"setenv bootdtb ${project_dir}/kernel/dtb;" \
		"setenv rootpath /home/projects/${project_dir}/rootfs;" \
		"setenv bootargs ${bootargs} " \
		"root=/dev/nfs ${mtdparts} " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} " \
		"ip=${ipaddr}:${serverip}:" \
		"${gatewayip}:${netmask}:${hostname}:eth0:off\0" \
	"net_nfs=echo Booting from network ...; " \
		"run net_args; " \
		"tftpboot ${kloadaddr} ${serverip}:${bootfile}; " \
		"tftpboot ${loadaddr} ${serverip}:${bootdtb}; " \
		"bootm ${kloadaddr} - ${loadaddr}\0"

/*
 * Variant 3 partition layout
 * chip-size = 512MiB
 *|         name |        size |           address area |
 *-------------------------------------------------------
 *|          spl | 128.000 KiB | 0x       0..0x   1ffff |
 *|  spl.backup1 | 128.000 KiB | 0x   20000..0x   3ffff |
 *|  spl.backup2 | 128.000 KiB | 0x   40000..0x   5ffff |
 *|  spl.backup3 | 128.000 KiB | 0x   60000..0x   7ffff |
 *|       u-boot |   1.875 MiB | 0x   80000..0x  25ffff |
 *|   uboot.env0 | 512.000 KiB | 0x  260000..0x  2Dffff |
 *|   uboot.env1 | 512.000 KiB | 0x  2E0000..0x  35ffff |
 *|       rootfs | 300.000 MiB | 0x  360000..0x12f5ffff |
 *|      mtdoops | 512.000 KiB | 0x12f60000..0x12fdffff |
 *|configuration | 104.125 MiB | 0x12fe0000..0x1fffffff |
 *-------------------------------------------------------

					"mtdparts=omap2-nand.0:" \
					"128k(spl),"		\
					"128k(spl.backup1),"	\
					"128k(spl.backup2),"	\
					"128k(spl.backup3),"	\
					"1920k(u-boot),"	\
					"512k(u-boot.env0),"	\
					"512k(u-boot.env1),"	\
					"300m(rootfs),"		\
					"512k(mtdoops),"	\
					"-(configuration)"

 */

#define CFG_SYS_NAND_BASE		(0x08000000)	/* physical address */
							/* to access nand at */
							/* CS0 */
#endif

#endif	/* ! __CONFIG_SIEMENS_AM33X_COMMON_H */
