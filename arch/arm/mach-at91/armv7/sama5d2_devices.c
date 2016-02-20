/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/sama5d2.h>

char *get_cpu_name()
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sama5d2()) {
		switch (extension_id) {
		case ARCH_EXID_SAMA5D21CU:
			return "SAMA5D21";
		case ARCH_EXID_SAMA5D22CU:
			return "SAMA5D22-CU";
		case ARCH_EXID_SAMA5D22CN:
			return "SAMA5D22-CN";
		case ARCH_EXID_SAMA5D23CU:
			return "SAMA5D23-CU";
		case ARCH_EXID_SAMA5D24CX:
			return "SAMA5D24-CX";
		case ARCH_EXID_SAMA5D24CU:
			return "SAMA5D24-CU";
		case ARCH_EXID_SAMA5D26CU:
			return "SAMA5D26-CU";
		case ARCH_EXID_SAMA5D27CU:
			return "SAMA5D27-CU";
		case ARCH_EXID_SAMA5D27CN:
			return "SAMA5D27-CN";
		case ARCH_EXID_SAMA5D28CU:
			return "SAMA5D28-CU";
		case ARCH_EXID_SAMA5D28CN:
			return "SAMA5D28-CN";
		}
	}

	return "Unknown CPU type";
}

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
void at91_udp_hw_init(void)
{
	at91_upll_clk_enable();

	at91_periph_clk_enable(ATMEL_ID_UDPHS);
}
#endif
