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
	sg_set_pinsel(70, 3);	/* HDDOUT0 -> TXD0 */
	sg_set_pinsel(71, 3);	/* HSDOUT1 -> RXD0 */

	sg_set_pinsel(114, 0);	/* TXD1 -> TXD1 */
	sg_set_pinsel(115, 0);	/* RXD1 -> RXD1 */

	sg_set_pinsel(112, 1);	/* SBO1 -> TXD2 */
	sg_set_pinsel(113, 1);	/* SBI1 -> RXD2 */

	sg_set_pinsel(110, 1);	/* SBO0 -> TXD3 */
	sg_set_pinsel(111, 1);	/* SBI0 -> RXD3 */
#endif
}
