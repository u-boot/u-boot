// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Microchip Technology, Inc.
 */

#include <asm/arch/sama7d65.h>

char *get_cpu_name(void)
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sama7d65())
		switch (extension_id) {
		case ARCH_EXID_SAMA7D65:
			return "SAMA7D65";
		case ARCH_EXID_SAMA7D65_DD2:
			return "SAMA7D65 DDR2";
		case ARCH_EXID_SAMA7D65_D1G:
			return "SAMA7D65 1Gb DDR3L SiP";
		case ARCH_EXID_SAMA7D65_D2G:
			return "SAMA7D65 2Gb DDR3L SiP";
		case ARCH_EXID_SAMA7D65_D4G:
			return "SAMA7D65 4Gb DDR3L SiP";
		case ARCH_EXID_SAMA7D65_TA:
			return "SAMA7D65 TA1000 SiP";
		default:
			return "Unknown CPU type";
		}
	else
		return "Unknown CPU type10";
}
