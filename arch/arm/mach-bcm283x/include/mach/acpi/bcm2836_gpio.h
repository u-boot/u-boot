/* SPDX-License-Identifier: BSD-2-Clause-Patent */
/**
 *
 *  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
 *  Copyright (c) 2018, Andrei Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 **/

#include <asm/arch/acpi/bcm2836.h>

#ifndef __BCM2836_GPIO_H__
#define __BCM2836_GPIO_H__

#define GPIO_OFFSET        0x00200000
#define GPIO_BASE_ADDRESS  (BCM2836_SOC_REGISTERS + GPIO_OFFSET)
#define GPIO_LENGTH        0x000000B4

#endif /* __BCM2836_GPIO_H__ */
