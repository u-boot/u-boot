/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Dummy header file to enable CONFIG_OF_CONTROL.
 * If CONFIG_OF_CONTROL is enabled, lib/fdtdec.c is compiled.
 * It includes <asm/arch/gpio.h> via <asm/gpio.h>, so those SoCs that enable
 * OF_CONTROL must have arch/gpio.h.
 */

#ifndef __ASM_ARCH_MX85XX_GPIO_H
#define __ASM_ARCH_MX85XX_GPIO_H

#include <asm/mpc85xx_gpio.h>

#endif
