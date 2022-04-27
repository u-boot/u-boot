/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor
 * Copyright 2019-2021 NXP
 */

#ifndef __LS1046A_COMMON_H
#define __LS1046A_COMMON_H

/* SPL build */
#ifdef CONFIG_SPL_BUILD
#define SPL_NO_QBMAN
#define SPL_NO_FMAN
#define SPL_NO_ENV
#define SPL_NO_MISC
#define SPL_NO_QSPI
#define SPL_NO_USB
#define SPL_NO_SATA
#endif
#if defined(CONFIG_SPL_BUILD) && \
	(defined(CONFIG_NAND_BOOT) || defined(CONFIG_QSPI_BOOT))
#define SPL_NO_MMC
#endif
#if defined(CONFIG_SPL_BUILD)		&& \
	!defined(CONFIG_SPL_FSL_LS_PPA)
#define SPL_NO_IFC
#endif

#include <asm/arch/config.h>
#include <asm/arch/stream_id_lsch2.h>

/* Link Definitions */
#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_FSL_OCRAM_BASE + 0xfff0)
#endif

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE      0x880000000ULL

#define CPU_RELEASE_ADDR               secondary_boot_addr

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK          (get_serial_clock())

/* SD boot SPL */
#ifdef CONFIG_SD_BOOT
#define CONFIG_SPL_MAX_SIZE		0x1f000		/* 124 KiB */
#define CONFIG_SPL_STACK		0x10020000
#define CONFIG_SPL_PAD_TO		0x21000		/* 132 KiB */
#define CONFIG_SPL_BSS_START_ADDR	0x8f000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

#ifdef CONFIG_NXP_ESBC
#define CONFIG_U_BOOT_HDR_SIZE				(16 << 10)
/*
 * HDR would be appended at end of image and copied to DDR along
 * with U-Boot image. Here u-boot max. size is 512K. So if binary
 * size increases then increase this size in case of secure boot as
 * it uses raw u-boot image instead of fit image.
 */
#define CONFIG_SYS_MONITOR_LEN		(0x100000 + CONFIG_U_BOOT_HDR_SIZE)
#else
#define CONFIG_SYS_MONITOR_LEN		0x100000
#endif /* ifdef CONFIG_NXP_ESBC */
#endif

#if defined(CONFIG_QSPI_BOOT) && defined(CONFIG_SPL)
#define CONFIG_SPL_TARGET		"spl/u-boot-spl.pbl"
#define CONFIG_SPL_MAX_SIZE		0x1f000
#define CONFIG_SPL_STACK		0x10020000
#define CONFIG_SPL_PAD_TO		0x20000
#define CONFIG_SPL_BSS_START_ADDR	0x8f000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000
#define CONFIG_SYS_MONITOR_LEN		0x100000
#endif

/* NAND SPL */
#ifdef CONFIG_NAND_BOOT
#define CONFIG_SPL_PBL_PAD

#define CONFIG_SPL_MAX_SIZE		0x17000		/* 90 KiB */
#define CONFIG_SPL_STACK		0x1001f000
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_SPL_BSS_START_ADDR	0x8f000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000
#define CONFIG_SYS_MONITOR_LEN		0xa0000
#endif

/* GPIO */

/* I2C */

/* PCIe */
#define CONFIG_PCIE1		/* PCIE controller 1 */
#define CONFIG_PCIE2		/* PCIE controller 2 */
#define CONFIG_PCIE3		/* PCIE controller 3 */

#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif

/* SATA */
#ifndef SPL_NO_SATA
#define CONFIG_SYS_SATA				AHCI_BASE_ADDR
#endif

/* FMan ucode */
#ifndef SPL_NO_FMAN
#define CONFIG_SYS_DPAA_FMAN
#ifdef CONFIG_SYS_DPAA_FMAN
#define CONFIG_SYS_FM_MURAM_SIZE	0x60000
#endif
#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)
#endif

/* Miscellaneous configurable options */

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(SCSI, scsi, 0) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#endif

#if defined(CONFIG_TARGET_LS1046AFRWY)
#define LS1046A_BOOT_SRC_AND_HDR\
	"boot_scripts=ls1046afrwy_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1046afrwy_bs.out\0"
#elif defined(CONFIG_TARGET_LS1046AQDS)
#define LS1046A_BOOT_SRC_AND_HDR\
	"boot_scripts=ls1046aqds_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1046aqds_bs.out\0"
#else
#define LS1046A_BOOT_SRC_AND_HDR\
	"boot_scripts=ls1046ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1046ardb_bs.out\0"
#endif
#ifndef SPL_NO_MISC
/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"bootm_size=0x10000000\0"		\
	"kernel_addr=0x61000000\0"              \
	"scriptaddr=0x80000000\0"               \
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"         \
	"kernelheader_addr_r=0x80200000\0"      \
	"load_addr=0xa0000000\0"            \
	"kernel_addr_r=0x81000000\0"            \
	"fdt_addr_r=0x90000000\0"               \
	"ramdisk_addr_r=0xa0000000\0"           \
	"kernel_start=0x1000000\0"		\
	"kernelheader_start=0x600000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernelheader_size=0x40000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernel_size_sd=0x14000\0"		\
	"kernelhdr_addr_sd=0x3000\0"		\
	"kernelhdr_size_sd=0x10\0"		\
	"console=ttyS0,115200\0"                \
	 CONFIG_MTDPARTS_DEFAULT "\0"		\
	BOOTENV					\
	LS1046A_BOOT_SRC_AND_HDR		\
	"scan_dev_for_boot_part="               \
		"part list ${devtype} ${devnum} devplist; "   \
		"env exists devplist || setenv devplist 1; "  \
		"for distro_bootpart in ${devplist}; do "     \
		  "if fstype ${devtype} "                  \
			"${devnum}:${distro_bootpart} "      \
			"bootfstype; then "                  \
			"run scan_dev_for_boot; "            \
		  "fi; "                                   \
		"done\0"                                   \
	"boot_a_script="				  \
		"load ${devtype} ${devnum}:${distro_bootpart} "  \
			"${scriptaddr} ${prefix}${script}; "    \
		"env exists secureboot && load ${devtype} "     \
			"${devnum}:${distro_bootpart} "		\
			"${scripthdraddr} ${prefix}${boot_script_hdr}; " \
			"env exists secureboot "	\
			"&& esbc_validate ${scripthdraddr};"	\
		"source ${scriptaddr}\0"	  \
	"qspi_bootcmd=echo Trying load from qspi..;"      \
		"sf probe && sf read $load_addr "         \
		"$kernel_start $kernel_size; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_start "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"		\
	"nand_bootcmd=echo Trying load from nand..;"      \
		"nand info; nand read $load_addr "         \
		"$kernel_start $kernel_size; env exists secureboot "	\
		"&& nand read $kernelheader_addr_r $kernelheader_start " \
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"		\
	"nor_bootcmd=echo Trying load from nor..;"	\
		"cp.b $kernel_addr $load_addr "		\
		"$kernel_size; env exists secureboot "	\
		"&& cp.b $kernelheader_addr $kernelheader_addr_r "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"	\
	"sd_bootcmd=echo Trying load from SD ..;"	\
		"mmcinfo; mmc read $load_addr "		\
		"$kernel_addr_sd $kernel_size_sd && "	\
		"env exists secureboot && mmc read $kernelheader_addr_r "		\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "		\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$board\0"

#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */

#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

#include <asm/arch/soc.h>

#endif /* __LS1046A_COMMON_H */
