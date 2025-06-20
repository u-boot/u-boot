// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries
 */

#include <asm/arch/at91_common.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

unsigned int get_chip_id(void)
{
	/* The 0x40 is the offset of cidr in DBGU */
	return readl(ATMEL_BASE_DBGU + 0x40);
}

unsigned int get_extension_chip_id(void)
{
	/* The 0x44 is the offset of exid in DBGU */
	return readl(ATMEL_BASE_DBGU + 0x44);
}

char *get_cpu_name(void)
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sam9x7()) {
		switch (extension_id) {
		case ARCH_EXID_SAM9X70:
			return "SAM9X70";
		case ARCH_EXID_SAM9X72:
			return "SAM9X72";
		case ARCH_EXID_SAM9X75:
			return "SAM9X75";
		case ARCH_EXID_SAM9X75_D1M:
			return "SAM9X75 16MB DDR2 SiP";
		case ARCH_EXID_SAM9X75_D5M:
			return "SAM9X75 64MB DDR2 SiP";
		case ARCH_EXID_SAM9X75_D1G:
			return "SAM9X75 125MB DDR3L SiP";
		case ARCH_EXID_SAM9X75_D2G:
			return "SAM9X75 250MB DDR3L SiP";
		default:
			return "Unknown CPU type";
		}
	} else {
		return "Unknown CPU type";
	}
}
