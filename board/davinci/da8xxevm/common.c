/*
 * Miscellaneous DA8XX functions.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include "common.h"

#ifndef CONFIG_USE_IRQ
void irq_init(void)
{
	/*
	 * Mask all IRQs by clearing the global enable and setting
	 * the enable clear for all the 90 interrupts.
	 */

	writel(0, &davinci_aintc_regs->ger);

	writel(0, &davinci_aintc_regs->hier);

	writel(0xffffffff, &davinci_aintc_regs->ecr1);
	writel(0xffffffff, &davinci_aintc_regs->ecr2);
	writel(0xffffffff, &davinci_aintc_regs->ecr3);
}
#endif

/*
 * Enable PSC for various peripherals.
 */
int da8xx_configure_lpsc_items(const struct lpsc_resource *item,
				    const int n_items)
{
	int i;

	for (i = 0; i < n_items; i++)
		lpsc_on(item[i].lpsc_no);

	return 0;
}
