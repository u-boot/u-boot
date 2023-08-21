/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM62Ax SoC family
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_AM62AX_EVM_H
#define __CONFIG_AM62AX_EVM_H

#include <linux/sizes.h>
#include <env/ti/mmc.h>
#include <env/ti/k3_dfu.h>

/* DDR Configuration */
#define CFG_SYS_SDRAM_BASE1		0x880000000


/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM62A7_EVM_H */
