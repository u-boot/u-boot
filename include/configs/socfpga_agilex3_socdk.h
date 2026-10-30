/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 */

#ifndef __CONFIG_SOCFPGA_AGILEX3_H__
#define __CONFIG_SOCFPGA_AGILEX3_H__

/*
 * Agilex3 QSPI layout is 12 MiB u-boot + 52 MiB root. Override the shared
 * SoC64 66m/190m bootcmd so other boards keep the default path.
 */
#define AGILEX3_BOOTENV_UNUSED(...)
#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	AGILEX3_BOOTENV_UNUSED(devtypeu, instance) \
	"bootcmd_" #devtypel "=ubi detach; sf probe && " \
	"setenv mtdids 'nor0=nor0,nand0=nand.0' && " \
	"setenv mtdparts 'mtdparts=nor0:12m(u-boot),52m(root); " \
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

#include <configs/socfpga_soc64_common.h>

#endif	/* __CONFIG_SOCFPGA_AGILEX3_H__ */
