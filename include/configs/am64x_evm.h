/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM642 SoC family
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 */

#ifndef __CONFIG_AM642_EVM_H
#define __CONFIG_AM642_EVM_H

#include <linux/sizes.h>
#include <config_distro_bootcmd.h>
#include <environment/ti/mmc.h>
#include <asm/arch/am64_hardware.h>
#include <environment/ti/k3_dfu.h>

/* DDR Configuration */
#define CFG_SYS_SDRAM_BASE1		0x880000000

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM642_EVM_H */
