/*
 * (C) Copyright 2009 ST-Ericsson
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

struct clkrst {
	unsigned int pcken;
	unsigned int pckdis;
	unsigned int kcken;
	unsigned int kckdis;
};

static unsigned int clkrst_base[] = {
	U8500_CLKRST1_BASE,
	U8500_CLKRST2_BASE,
	U8500_CLKRST3_BASE,
	0,
	U8500_CLKRST5_BASE,
	U8500_CLKRST6_BASE,
	U8500_CLKRST7_BASE,	/* ED only */
};

/* Turn on peripheral clock at PRCC level */
void u8500_clock_enable(int periph, int cluster, int kern)
{
	struct clkrst *clkrst = (struct clkrst *) clkrst_base[periph - 1];

	if (kern != -1)
		writel(1 << kern, &clkrst->kcken);

	if (cluster != -1)
		writel(1 << cluster, &clkrst->pcken);
}

void db8500_clocks_init(void)
{
	/*
	 * Enable all clocks. This is u-boot, we can enable it all. There is no
	 * powersave in u-boot.
	 */

	u8500_clock_enable(1, 9, -1); /* GPIO0 */
	u8500_clock_enable(2, 11, -1);/* GPIO1 */
	u8500_clock_enable(3, 8, -1); /* GPIO2 */
	u8500_clock_enable(5, 1, -1); /* GPIO3 */
	u8500_clock_enable(3, 6, 6);  /* UART2 */
	u8500_clock_enable(3, 3, 3);  /* I2C0 */
	u8500_clock_enable(1, 5, 5);  /* SDI0 */
	u8500_clock_enable(2, 4, 2);  /* SDI4 */
	u8500_clock_enable(6, 6, -1); /* MTU0 */
	u8500_clock_enable(3, 4, 4);  /* SDI2 */

	/*
	 * Enabling clocks for all devices which are AMBA devices in the
	 * kernel.  Otherwise they will not get probe()'d because the
	 * peripheral ID register will not be powered.
	 */

	/* XXX: some of these differ between ED/V1 */

	u8500_clock_enable(1, 1, 1);  /* UART1 */
	u8500_clock_enable(1, 0, 0);  /* UART0 */
	u8500_clock_enable(3, 2, 2);  /* SSP1 */
	u8500_clock_enable(3, 1, 1);  /* SSP0 */
	u8500_clock_enable(2, 8, -1); /* SPI0 */
	u8500_clock_enable(2, 5, 3);  /* MSP2 */
}
