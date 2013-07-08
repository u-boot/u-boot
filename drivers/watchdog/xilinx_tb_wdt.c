/*
 * Copyright (c) 2011-2013 Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
