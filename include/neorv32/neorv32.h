// SPDX-License-Identifier: GPL-2.0+
/*
 * NEORV32 base definitions (subset)
 */

#ifndef __NEORV32_H
#define __NEORV32_H

#include <linux/types.h>

#define IO_BASE_ADDRESS      (0xFFE00000U)
#define NEORV32_BOOTROM_BASE (0xFFE00000U)
#define NEORV32_CLINT_BASE   (0xFFF40000U)
#define NEORV32_UART0_BASE   (0xFFF50000U)
#define NEORV32_UART1_BASE   (0xFFF60000U)
#define NEORV32_SPI_BASE     (0xFFF80000U)
#define NEORV32_TWI_BASE     (0xFFF90000U)
#define NEORV32_SYSINFO_BASE (0xFFFE0000U)

/* SYSINFO register offsets */
#define NEORV32_SYSINFO_CLK  0x00

/* Clock prescaler select (relative to processor clock) */
enum NEORV32_CLOCK_PRSC_enum {
	CLK_PRSC_2    = 0,
	CLK_PRSC_4    = 1,
	CLK_PRSC_8    = 2,
	CLK_PRSC_64   = 3,
	CLK_PRSC_128  = 4,
	CLK_PRSC_1024 = 5,
	CLK_PRSC_2048 = 6,
	CLK_PRSC_4096 = 7,
};

#endif /* __NEORV32_H */
