/*
 * Copyright (c) 2011-2013 Xilinx Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/microblaze_intc.h>
#include <asm/processor.h>
#include <watchdog.h>

#define XWT_CSR0_WRS_MASK	0x00000008 /* Reset status Mask */
#define XWT_CSR0_WDS_MASK	0x00000004 /* Timer state Mask */
#define XWT_CSR0_EWDT1_MASK	0x00000002 /* Enable bit 1 Mask*/
#define XWT_CSRX_EWDT2_MASK	0x00000001 /* Enable bit 2 Mask */

struct watchdog_regs {
	u32 twcsr0; /* 0x0 */
	u32 twcsr1; /* 0x4 */
	u32 tbr; /* 0x8 */
};

static struct watchdog_regs *watchdog_base =
			(struct watchdog_regs *)CONFIG_WATCHDOG_BASEADDR;

void hw_watchdog_reset(void)
{
	u32 reg;

	/* Read the current contents of TCSR0 */
	reg = readl(&watchdog_base->twcsr0);

	/* Clear the watchdog WDS bit */
	if (reg & (XWT_CSR0_EWDT1_MASK | XWT_CSRX_EWDT2_MASK))
		writel(reg | XWT_CSR0_WDS_MASK, &watchdog_base->twcsr0);
}

void hw_watchdog_disable(void)
{
	u32 reg;

	/* Read the current contents of TCSR0 */
	reg = readl(&watchdog_base->twcsr0);

	writel(reg & ~XWT_CSR0_EWDT1_MASK, &watchdog_base->twcsr0);
	writel(~XWT_CSRX_EWDT2_MASK, &watchdog_base->twcsr1);

	puts("Watchdog disabled!\n");
}

static void hw_watchdog_isr(void *arg)
{
	hw_watchdog_reset();
}

int hw_watchdog_init(void)
{
	int ret;

	writel((XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK | XWT_CSR0_EWDT1_MASK),
	       &watchdog_base->twcsr0);
	writel(XWT_CSRX_EWDT2_MASK, &watchdog_base->twcsr1);

	ret = install_interrupt_handler(CONFIG_WATCHDOG_IRQ,
						hw_watchdog_isr, NULL);
	if (ret)
		return 1;

	return 0;
}
