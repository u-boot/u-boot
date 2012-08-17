/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
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
