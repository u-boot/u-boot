/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright 2019-2021 Traverse Technologies
 */

#ifndef __TEN64_H
#define __TEN64_H

#include "ls1088a_common.h"


#define CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000

#define QSPI_NOR_BOOTCOMMAND	"run distro_bootcmd"
#define SD_BOOTCOMMAND		"run distro_bootcmd"

#define QSPI_MC_INIT_CMD				\
	"sf probe 0:0 && sf read 0x80000000 0x300000 0x200000 &&"	\
	"sf read 0x80200000 0x5C0000 0x40000 &&"				\
	"fsl_mc start mc 0x80000000 0x80200000 && " \
	"sf read 0x80300000 0x580000 0x40000 && fsl_mc lazyapply DPL 0x80300000\0"
#define SD_MC_INIT_CMD				\
	"mmcinfo; fatload mmc 0 0x80000000 mcfirmware/mc_ls1088a.itb; "\
	"fatload mmc 0 0x80200000 dpaa2config/dpc.0x1D-0x0D.dtb; "\
	"fsl_mc start mc 0x80000000 0x80200000\0"

#define BOOT_TARGET_DEVICES(func) \
	func(NVME, nvme, 0) \
	func(USB, usb, 0) \
	func(MMC, mmc, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, 0) \
	func(PXE, pxe, 0)
#include <config_distro_bootcmd.h>

#undef CONFIG_EXTRA_ENV_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS \
	"BOARD=ten64\0"					\
	"fdt_addr_r=0x90000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"kernel_addr_r=0x81000000\0"		\
	"load_addr=0xa0000000\0"		\
	BOOTENV \
	"load_efi_dtb=mtd read devicetree $fdt_addr_r && fdt addr $fdt_addr_r && " \
	"fdt resize && fdt boardsetup\0" \
	"bootcmd_recovery=mtd read recovery 0xa0000000; mtd read dpl 0x80100000 && " \
	"fsl_mc apply DPL 0x80100000 && bootm 0xa0000000#ten64\0"

#endif /* __TEN64_H */
