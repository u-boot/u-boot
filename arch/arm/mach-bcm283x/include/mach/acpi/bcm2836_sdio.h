/* SPDX-License-Identifier: BSD-2-Clause-Patent */
/**
 *
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 **/

#include <asm/arch/acpi/bcm2836.h>

#ifndef __BCM2836_SDIO_H__
#define __BCM2836_SDIO_H__

// MMC/SD/SDIO1 register definitions.
#define MMCHS1_OFFSET     0x00300000
#define MMCHS2_OFFSET     0x00340000
#define MMCHS1_BASE       (BCM2836_SOC_REGISTERS + MMCHS1_OFFSET)
#define MMCHS2_BASE       (BCM2836_SOC_REGISTERS + MMCHS2_OFFSET)
#define MMCHS1_LENGTH     0x00000100
#define MMCHS2_LENGTH     0x00000100

#endif /* __BCM2836_SDIO_H__ */
