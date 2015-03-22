/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2015      Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/sg-regs.h>

void early_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_UNIPHIER_SERIAL
	sg_set_pinsel(85, 1);	/* HSDOUT3 -> RXD0 */
	sg_set_pinsel(88, 1);	/* HDDOUT6 -> TXD0 */

	sg_set_pinsel(69, 23);	/* PCIOWR -> TXD1 */
	sg_set_pinsel(70, 23);	/* PCIORD -> RXD1 */

	sg_set_pinsel(128, 13);	/* XIRQ6 -> TXD2 */
	sg_set_pinsel(129, 13);	/* XIRQ7 -> RXD2 */

	sg_set_pinsel(110, 1);	/* SBO0 -> TXD3 */
	sg_set_pinsel(111, 1);	/* SBI0 -> RXD3 */
#endif
}
