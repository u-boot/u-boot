// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/rcar-common/gen4-common.c
 *
 * Copyright (C) 2021-2024 Renesas Electronics Corp.
 */

#include <asm/arch/renesas.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <linux/errno.h>

#define RST_BASE	0xE6160000 /* Domain0 */
#define RST_WDTRSTCR	(RST_BASE + 0x10)
#define RST_RWDT	0xA55A8002

DECLARE_GLOBAL_DATA_PTR;

static void init_generic_timer(void)
{
	const u32 freq = CONFIG_SYS_CLK_FREQ;

	/* Update memory mapped and register based freqency */
	asm volatile ("msr cntfrq_el0, %0" :: "r" (freq));
	writel(freq, CNTFID0);

	/* Enable counter */
	setbits_le32(CNTCR_BASE, CNTCR_EN);
}

static void init_gic_v3(void)
{
	/* GIC v3 power on */
	writel(BIT(1), GICR_LPI_PWRR);

	/* Wait till the WAKER_CA_BIT changes to 0 */
	clrbits_le32(GICR_LPI_WAKER, BIT(1));
	while (readl(GICR_LPI_WAKER) & BIT(2))
		;

	writel(0xffffffff, GICR_SGI_BASE + GICR_IGROUPR0);
}

void s_init(void)
{
	if (current_el() == 3)
		init_generic_timer();
}

int board_early_init_f(void)
{
	/* Unlock CPG access */
	writel(0x5A5AFFFF, CPGWPR);
	writel(0xA5A50000, CPGWPCR);

	return 0;
}

int board_init(void)
{
	if (current_el() != 3)
		return 0;
	init_gic_v3();

	/* Enable RWDT reset on V3U in EL3 */
	if (IS_ENABLED(CONFIG_R8A779A0) &&
	    renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A779A0) {
		writel(RST_RWDT, RST_WDTRSTCR);
	}

	return 0;
}
