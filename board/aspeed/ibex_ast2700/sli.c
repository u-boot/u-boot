// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#include <asm/io.h>
#include <asm/arch/sli.h>
#include <asm/arch/scu.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

#define SLI_POLL_TIMEOUT_US	100

static void sli_clear_interrupt_status(uint32_t base)
{
	writel(-1, (void *)base + SLI_INTR_STATUS);
}

static int sli_wait(uint32_t base, uint32_t mask)
{
	uint32_t value;

	sli_clear_interrupt_status(base);

	do {
		value = readl((void *)base + SLI_INTR_STATUS);
		if (value & SLI_INTR_RX_ERRORS)
			return -1;
	} while ((value & mask) != mask);

	return 0;
}

static int sli_wait_suspend(uint32_t base)
{
	return sli_wait(base, SLI_INTR_TX_SUSPEND | SLI_INTR_RX_SUSPEND);
}

/*
 * CPU die  --- downstream pads ---> I/O die
 * CPU die  <--- upstream pads ----- I/O die
 *
 * US/DS PAD[3:0] : SLIM[3:0]
 * US/DS PAD[5:4] : SLIH[1:0]
 * US/DS PAD[7:6] : SLIV[1:0]
 */
int sli_init(void)
{
	uint32_t value;

	/* The following training sequence is designed for AST2700A0 */
	value = FIELD_GET(SCU1_REVISION_HWID, readl(SCU1_REVISION));
	if (value)
		return 0;

	/* Return if SLI had been calibrated */
	value = readl((void *)SLIH_IOD_BASE + SLI_CTRL_III);
	value = FIELD_GET(SLI_CLK_SEL, value);
	if (value) {
		debug("SLI has been initialized\n");
		return 0;
	}

	/* 25MHz PAD delay for AST2700A0 */
	value = SLI_RX_PHY_LAH_SEL_NEG | SLI_TRANS_EN | SLI_CLEAR_BUS;
	writel(value, (void *)SLIH_IOD_BASE + SLI_CTRL_I);
	writel(value, (void *)SLIM_IOD_BASE + SLI_CTRL_I);
	writel(value | SLIV_RAW_MODE, (void *)SLIV_IOD_BASE + SLI_CTRL_I);
	sli_wait_suspend(SLIH_IOD_BASE);
	sli_wait_suspend(SLIH_CPU_BASE);

	return 0;
}
