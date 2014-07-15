/*
 * Keystone2: Architecture initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/arch/msmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>

void chip_configuration_unlock(void)
{
	__raw_writel(KS2_KICK0_MAGIC, KS2_KICK0);
	__raw_writel(KS2_KICK1_MAGIC, KS2_KICK1);
}

int arch_cpu_init(void)
{
	chip_configuration_unlock();
	icache_enable();

	msmc_share_all_segments(8);  /* TETRIS */
	msmc_share_all_segments(9);  /* NETCP */
	msmc_share_all_segments(10); /* QM PDSP */
	msmc_share_all_segments(11); /* PCIE 0 */
#ifdef CONFIG_SOC_K2E
	msmc_share_all_segments(13); /* PCIE 1 */
#endif

	/*
	 * just initialise the COM2 port so that TI specific
	 * UART register PWREMU_MGMT is initialized. Linux UART
	 * driver doesn't handle this.
	 */
	NS16550_init((NS16550_t)(CONFIG_SYS_NS16550_COM2),
		     CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);

	return 0;
}

void reset_cpu(ulong addr)
{
	volatile u32 *rstctrl = (volatile u32 *)(KS2_RSTCTRL);
	u32 tmp;

	tmp = *rstctrl & KS2_RSTCTRL_MASK;
	*rstctrl = tmp | KS2_RSTCTRL_KEY;

	*rstctrl &= KS2_RSTCTRL_SWRST;

	for (;;)
		;
}

void enable_caches(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
#endif
}
