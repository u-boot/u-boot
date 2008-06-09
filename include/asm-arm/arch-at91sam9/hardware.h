/*
 * [origin: Linux kernel include/asm-arm/arch-at91/hardware.h]
 *
 *  Copyright (C) 2003 SAN People
 *  Copyright (C) 2003 ATMEL
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>

#if defined(CONFIG_AT91RM9200)
#include <asm/arch/at91rm9200.h>
#elif defined(CONFIG_AT91SAM9260)
#include <asm/arch/at91sam9260.h>
#define AT91_BASE_EMAC	AT91SAM9260_BASE_EMAC
#define AT91_BASE_SPI	AT91SAM9260_BASE_SPI0
#define AT91_ID_UHP	AT91SAM9260_ID_UHP
#define AT91_PMC_UHP	AT91SAM926x_PMC_UHP
#elif defined(CONFIG_AT91SAM9261)
#include <asm/arch/at91sam9261.h>
#define AT91_BASE_SPI	AT91SAM9261_BASE_SPI0
#define AT91_ID_UHP	AT91SAM9261_ID_UHP
#define AT91_PMC_UHP	AT91SAM926x_PMC_UHP
#elif defined(CONFIG_AT91SAM9263)
#include <asm/arch/at91sam9263.h>
#define AT91_BASE_EMAC	AT91SAM9263_BASE_EMAC
#define AT91_BASE_SPI	AT91SAM9263_BASE_SPI0
#define AT91_ID_UHP	AT91SAM9263_ID_UHP
#define AT91_PMC_UHP	AT91SAM926x_PMC_UHP
#elif defined(CONFIG_AT91SAM9RL)
#include <asm/arch/at91sam9rl.h>
#define AT91_BASE_SPI	AT91SAM9RL_BASE_SPI
#define AT91_ID_UHP	AT91SAM9RL_ID_UHP
#elif defined(CONFIG_AT91CAP9)
#include <asm/arch/at91cap9.h>
#define AT91_BASE_EMAC	AT91CAP9_BASE_EMAC
#define AT91_BASE_SPI	AT91CAP9_BASE_SPI0
#define AT91_ID_UHP	AT91CAP9_ID_UHP
#define AT91_PMC_UHP	AT91CAP9_PMC_UHP
#elif defined(CONFIG_AT91X40)
#include <asm/arch/at91x40.h>
#else
#error "Unsupported AT91 processor"
#endif

#endif
