/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2017-2024 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#ifndef __CONFIG_SOCFPGA_SOC64_COMMON_H__
#define __CONFIG_SOCFPGA_SOC64_COMMON_H__

#include <asm/arch/base_addr_soc64.h>
#include <asm/arch/handoff_soc64.h>
#include <linux/stringify.h>

/*
 * U-Boot general configurations
 */
/* sysmgr.boot_scratch_cold4 & 5 (64bit) will be used for PSCI_CPU_ON call */
#define CPU_RELEASE_ADDR		0xFFD12210

/*
 * Share sysmgr.boot_scratch_cold6 & 7 (64bit) with VBAR_LE3_BASE_ADDR
 * Indicate L2 reset is done. HPS should trigger warm reset via RMR_EL3.
 */
#define L2_RESET_DONE_REG		0xFFD12218

/* sysmgr.boot_scratch_cold8 bit 17 (1bit) will be used to check whether CPU0
 * is being powered off/on from kernel
 */
#define BOOT_SCRATCH_COLD8		0xFFD12220

/* Magic word to indicate L2 reset is completed */
#define L2_RESET_DONE_STATUS		0x1228E5E7

/*
 * U-Boot console configurations
 */

/* Extend size of kernel image for uncompression */

/*
 * U-Boot run time memory configurations
 */
#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)
#define CFG_SYS_INIT_RAM_ADDR	0x0
#define CFG_SYS_INIT_RAM_SIZE	0x80000
#else
#define CFG_SYS_INIT_RAM_ADDR	0xFFE00000
#define CFG_SYS_INIT_RAM_SIZE	0x40000
#endif

/*
 * U-Boot environment configurations
 */

#define CFG_SYS_NAND_U_BOOT_SIZE	(1 * 1024 * 1024)
#define CFG_SYS_NAND_U_BOOT_DST	CONFIG_TEXT_BASE

/*
 * Environment variable
 */
#if IS_ENABLED(CONFIG_DISTRO_DEFAULTS)
#if IS_ENABLED(CONFIG_CMD_MMC)
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#if IS_ENABLED(CONFIG_CMD_SF)
#define BOOT_TARGET_DEVICES_QSPI(func) func(QSPI, qspi, na)
#else
#define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=ubi detach; sf probe && " \
	"setenv mtdids 'nor0=nor0,nand0=nand.0' && " \
	"setenv mtdparts 'mtdparts=nor0:66m(u-boot),190m(root); " \
	"nand.0:2m(nand_uboot),500m(nand_root)' && " \
	"env select UBI; saveenv && " \
	"ubi part root && " \
	"if ubi part root && ubi readvol ${scriptaddr} script; " \
	"then echo QSPI: Running script from UBIFS; " \
	"elif sf read ${scriptaddr} ${qspiscriptaddr} ${scriptsize}; " \
	"then echo QSPI: Running script from JFFS2; fi; " \
	"echo QSPI: Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; " \
	"echo QSPI: SCRIPT FAILED: continuing...; ubi detach;\0"

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#if IS_ENABLED(CONFIG_CMD_NAND)
#define BOOT_TARGET_DEVICES_NAND(func)	func(NAND, nand, na)
#else
#define BOOT_TARGET_DEVICES_NAND(func)
#endif

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_nand=ubi detach && " \
	"setenv mtdids 'nor0=nor0,nand0=nand.0' && " \
	"setenv mtdparts 'mtdparts=nor0:66m(qspi_uboot),190m(qspi_root);" \
	"nand.0:2m(u-boot),500m(root)' && " \
	"env select UBI; saveenv && " \
	"ubi part root && " \
	"ubi readvol ${scriptaddr} script && " \
	"echo NAND: Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; " \
	"echo NAND: SCRIPT FAILED: continuing...; ubi detach;\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	"nand "

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_QSPI(func) \
	BOOT_TARGET_DEVICES_NAND(func)

#include <config_distro_bootcmd.h>

#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)

#define CFG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x82000000\0" \
	"fdt_addr_r=0x86000000\0" \
	"qspiscriptaddr=0x02110000\0" \
	"scriptsize=0x00010000\0" \
	"qspibootimageaddr=0x02120000\0" \
	"bootimagesize=0x03200000\0" \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"bootfile=" CONFIG_BOOTFILE "\0" \
	"mmcroot=/dev/mmcblk0p2\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"linux_qspi_enable=if sf probe; then " \
		"echo Enabling QSPI at Linux DTB...;" \
		"fdt addr ${fdt_addr}; fdt resize;" \
		"fdt set /soc/spi@108d2000 status okay;" \
		"if fdt set /clocks/qspi-clk clock-frequency" \
		" ${qspi_clock}; then echo QSPI clock frequency updated;" \
		" elif fdt set /soc/clkmgr/clocks/qspi_clk clock-frequency" \
		" ${qspi_clock}; then echo QSPI clock frequency updated;" \
		" else fdt set /clocks/qspi-clk clock-frequency" \
		" ${qspi_clock}; echo QSPI clock frequency updated; fi; fi\0" \
	"scriptaddr=0x81000000\0" \
	"scriptfile=boot.scr\0" \
	"socfpga_legacy_reset_compat=1\0" \
	"smc_fid_rd=0xC2000007\0" \
	"smc_fid_wr=0xC2000008\0" \
	"smc_fid_upd=0xC2000009\0 " \
	BOOTENV

#else

#define CFG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x2000000\0" \
	"fdt_addr_r=0x6000000\0" \
	"qspiscriptaddr=0x02110000\0" \
	"scriptsize=0x00010000\0" \
	"qspibootimageaddr=0x02120000\0" \
	"bootimagesize=0x03200000\0" \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"bootfile=" CONFIG_BOOTFILE  "\0" \
	"mmcroot=/dev/mmcblk0p2\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"linux_qspi_enable=if sf probe; then " \
		"echo Enabling QSPI at Linux DTB...;" \
		"fdt addr ${fdt_addr}; fdt resize;" \
		"fdt set /soc/spi@ff8d2000 status okay;" \
		"if fdt set /soc/clocks/qspi-clk clock-frequency" \
		" ${qspi_clock}; then echo QSPI clock frequency updated;" \
		" elif fdt set /soc/clkmgr/clocks/qspi_clk clock-frequency" \
		" ${qspi_clock}; then echo QSPI clock frequency updated;" \
		" else fdt set /clocks/qspi-clk clock-frequency" \
		" ${qspi_clock}; echo QSPI clock frequency updated; fi; fi\0" \
	"scriptaddr=0x05FF0000\0" \
	"scriptfile=boot.scr\0" \
	"nandroot=ubi0:rootfs\0" \
	"socfpga_legacy_reset_compat=1\0" \
	"smc_fid_rd=0xC2000007\0" \
	"smc_fid_wr=0xC2000008\0" \
	"smc_fid_upd=0xC2000009\0 " \
	BOOTENV
#endif /*#IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)*/

#else

#define CFG_EXTRA_ENV_SETTINGS \
	"kernel_comp_addr_r=0x9000000\0" \
	"kernel_comp_size=0x01000000\0" \
	"qspibootimageaddr=0x020E0000\0" \
	"qspifdtaddr=0x020D0000\0" \
	"bootimagesize=0x01F00000\0" \
	"fdtimagesize=0x00010000\0" \
	"qspiload=sf read ${loadaddr} ${qspibootimageaddr} ${bootimagesize};" \
		"sf read ${fdt_addr} ${qspifdtaddr} ${fdtimagesize}\0" \
	"qspiboot=setenv bootargs earlycon root=/dev/mtdblock1 rw " \
		"rootfstype=jffs2 rootwait;booti ${loadaddr} - ${fdt_addr}\0" \
	"qspifitload=sf read ${loadaddr} ${qspibootimageaddr} ${bootimagesize}\0" \
	"qspifitboot=setenv bootargs earlycon root=/dev/mtdblock1 rw " \
		"rootfstype=jffs2 rootwait;bootm ${loadaddr}\0" \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"bootfile=" CONFIG_BOOTFILE  "\0" \
	"fdt_addr=8000000\0" \
	"fdtimage=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"mmcroot=/dev/mmcblk0p2\0" \
	"mmcboot=setenv bootargs " CONFIG_BOOTARGS \
		" root=${mmcroot} rw rootwait;" \
		"booti ${loadaddr} - ${fdt_addr}\0" \
	"mmcload=mmc rescan;" \
		"load mmc 0:1 ${loadaddr} ${bootfile};" \
		"load mmc 0:1 ${fdt_addr} ${fdtimage}\0" \
	"mmcfitboot=setenv bootargs " CONFIG_BOOTARGS \
		" root=${mmcroot} rw rootwait;" \
		"bootm ${loadaddr}\0" \
	"mmcfitload=mmc rescan;" \
		"load mmc 0:1 ${loadaddr} ${bootfile}\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"linux_qspi_enable=if sf probe; then " \
		"echo Enabling QSPI at Linux DTB...;" \
		"fdt addr ${fdt_addr}; fdt resize;" \
		"fdt set /soc/spi@ff8d2000 status okay;" \
		"if fdt set /soc/clocks/qspi-clk clock-frequency" \
		" ${qspi_clock}; then echo QSPI clock frequency updated;" \
		" elif fdt set /soc/clkmgr/clocks/qspi_clk clock-frequency" \
		" ${qspi_clock}; then echo QSPI clock frequency updated;" \
		" else fdt set /clocks/qspi-clk clock-frequency" \
		" ${qspi_clock}; echo QSPI clock frequency updated; fi; fi\0" \
	"scriptaddr=0x02100000\0" \
	"scriptfile=u-boot.scr\0" \
	"fatscript=if fatload mmc 0:1 ${scriptaddr} ${scriptfile};" \
		   "then source ${scriptaddr}:script; fi\0" \
	"nandfitboot=setenv bootargs " CONFIG_BOOTARGS \
			" root=${nandroot} rw rootwait rootfstype=ubifs ubi.mtd=1; " \
			"bootm ${loadaddr}\0" \
	"nandfitload=ubi part root; ubi readvol ${loadaddr} kernel\0" \
	"socfpga_legacy_reset_compat=1\0" \
	"smc_fid_rd=0xC2000007\0" \
	"smc_fid_wr=0xC2000008\0" \
	"smc_fid_upd=0xC2000009\0 "
#endif /*#if IS_ENABLED(CONFIG_DISTRO_DEFAULTS)*/

/*
 * External memory configurations
 */
#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		(1 * 1024 * 1024 * 1024)
#define CFG_SYS_SDRAM_BASE		0x80000000
#else
#define PHYS_SDRAM_1			0x0
#define PHYS_SDRAM_1_SIZE		(1 * 1024 * 1024 * 1024)
#define CFG_SYS_SDRAM_BASE		0
#endif

/*
 * Serial / UART configurations
 */
#define CFG_SYS_NS16550_CLK		100000000

/*
 * SDMMC configurations
 */
/*
 * Flash configurations
 */

/*
 * L4 Watchdog
 */
#ifdef CONFIG_TARGET_SOCFPGA_STRATIX10
#ifndef __ASSEMBLY__
unsigned int cm_get_l4_sys_free_clk_hz(void);
#define CFG_DW_WDT_CLOCK_KHZ		(cm_get_l4_sys_free_clk_hz() / 1000)
#endif
#else
#define CFG_DW_WDT_CLOCK_KHZ		100000
#endif

/*
 * SPL memory layout
 *
 * On chip RAM
 * 0xFFE0_0000 ...... Start of OCRAM
 * SPL code, rwdata
 * empty space
 * 0xFFEx_xxxx ...... Top of stack (grows down)
 * 0xFFEy_yyyy ...... Global Data
 * 0xFFEz_zzzz ...... Malloc prior relocation (size CONFIG_SYS_MALLOC_F_LEN)
 * 0xFFE3_F000 ...... Hardware handdoff blob (size 4KB)
 * 0xFFE3_FFFF ...... End of OCRAM
 *
 * SDRAM
 * 0x0000_0000 ...... Start of SDRAM_1
 * unused / empty space for image loading
 * Size 64MB   ...... MALLOC (size CONFIG_SPL_SYS_MALLOC_SIZE)
 * Size 1MB    ...... BSS (size CONFIG_SPL_BSS_MAX_SIZE)
 * 0x8000_0000 ...... End of SDRAM_1 (assume 2GB)
 *
 */

#endif	/* __CONFIG_SOCFPGA_SOC64_COMMON_H__ */
