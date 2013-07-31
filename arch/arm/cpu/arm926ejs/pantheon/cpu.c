/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pantheon.h>

#define UARTCLK14745KHZ	(APBC_APBCLK | APBC_FNCLK | APBC_FNCLKSEL(1))
#define SET_MRVL_ID	(1<<8)
#define L2C_RAM_SEL	(1<<4)

int arch_cpu_init(void)
{
	u32 val;
	struct panthcpu_registers *cpuregs =
		(struct panthcpu_registers*) PANTHEON_CPU_BASE;

	struct panthapb_registers *apbclkres =
		(struct panthapb_registers*) PANTHEON_APBC_BASE;

	struct panthmpmu_registers *mpmu =
		(struct panthmpmu_registers*) PANTHEON_MPMU_BASE;

	struct panthapmu_registers *apmu =
		(struct panthapmu_registers *) PANTHEON_APMU_BASE;

	/* set SEL_MRVL_ID bit in PANTHEON_CPU_CONF register */
	val = readl(&cpuregs->cpu_conf);
	val = val | SET_MRVL_ID;
	writel(val, &cpuregs->cpu_conf);

	/* Turn on clock gating (PMUM_CCGR) */
	writel(0xFFFFFFFF, &mpmu->ccgr);

	/* Turn on clock gating (PMUM_ACGR) */
	writel(0xFFFFFFFF, &mpmu->acgr);

	/* Turn on uart2 clock */
	writel(UARTCLK14745KHZ, &apbclkres->uart0);

	/* Enable GPIO clock */
	writel(APBC_APBCLK, &apbclkres->gpio);

#ifdef CONFIG_I2C_MV
	/* Enable I2C clock */
	writel(APBC_RST | APBC_FNCLK | APBC_APBCLK, &apbclkres->twsi);
	writel(APBC_FNCLK | APBC_APBCLK, &apbclkres->twsi);
#endif

#ifdef CONFIG_MV_SDHCI
	/* Enable mmc clock */
	writel(APMU_PERI_CLK | APMU_AXI_CLK | APMU_PERI_RST | APMU_AXI_RST,
			&apmu->sd1);
	writel(APMU_PERI_CLK | APMU_AXI_CLK | APMU_PERI_RST | APMU_AXI_RST,
			&apmu->sd3);
#endif

	icache_enable();

	return 0;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	u32 id;
	struct panthcpu_registers *cpuregs =
		(struct panthcpu_registers*) PANTHEON_CPU_BASE;

	id = readl(&cpuregs->chip_id);
	printf("SoC:   PANTHEON 88AP%X-%X\n", (id & 0xFFF), (id >> 0x10));
	return 0;
}
#endif

#ifdef CONFIG_I2C_MV
void i2c_clk_enable(void)
{
}
#endif
