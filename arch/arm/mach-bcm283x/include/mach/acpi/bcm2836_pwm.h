/* SPDX-License-Identifier: BSD-2-Clause-Patent */
/**
 *
 *  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
 *
 **/

#include <asm/arch/acpi/bcm2836.h>

#ifndef __BCM2836_PWM_H__
#define __BCM2836_PWM_H__

/* PWM controller constants */

#define BCM2836_PWM_DMA_OFFSET                 0x00007B00
#define BCM2836_PWM_DMA_BASE_ADDRESS           (BCM2836_SOC_REGISTERS + BCM2836_PWM_DMA_OFFSET)
#define BCM2836_PWM_DMA_LENGTH                 0x00000100

#define BCM2836_PWM_CLK_OFFSET                 0x001010A0
#define BCM2836_PWM_CLK_BASE_ADDRESS           (BCM2836_SOC_REGISTERS + BCM2836_PWM_CLK_OFFSET)
#define BCM2836_PWM_CLK_LENGTH                 0x00000008

#define BCM2836_PWM_CTRL_OFFSET                0x0020C000
#define BCM2836_PWM_CTRL_BASE_ADDRESS          (BCM2836_SOC_REGISTERS + BCM2836_PWM_CTRL_OFFSET)
#define BCM2836_PWM_CTRL_LENGTH                0x00000028

#define BCM2836_PWM_BUS_BASE_ADDRESS           0x7E20C000
#define BCM2836_PWM_BUS_LENGTH                 0x00000028

#define BCM2836_PWM_CTRL_UNCACHED_BASE_ADDRESS 0xFF20C000
#define BCM2836_PWM_CTRL_UNCACHED_LENGTH       0x00000028

#endif /* __BCM2836_PWM_H__ */
