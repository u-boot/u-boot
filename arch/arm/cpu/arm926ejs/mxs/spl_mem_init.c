/*
 * Freescale i.MX28 RAM init
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

#include "mxs_init.h"

static uint32_t dram_vals[] = {
/*
 * i.MX28 DDR2 at 200MHz
 */
#if defined(CONFIG_MX28)
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000100, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00010101, 0x01010101,
	0x000f0f01, 0x0f02020a, 0x00000000, 0x00010101,
	0x00000100, 0x00000100, 0x00000000, 0x00000002,
	0x01010000, 0x05060302, 0x06005003, 0x0a0000c8,
	0x02009c40, 0x0000030c, 0x0036a609, 0x031a0612,
	0x02030202, 0x00c8001c, 0x00000000, 0x00000000,
	0x00012100, 0xffff0303, 0x00012100, 0xffff0303,
	0x00012100, 0xffff0303, 0x00012100, 0xffff0303,
	0x00000003, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000612, 0x01000F02,
	0x06120612, 0x00000200, 0x00020007, 0xf5014b27,
	0xf5014b27, 0xf5014b27, 0xf5014b27, 0x07000300,
	0x07000300, 0x07000300, 0x07000300, 0x00000006,
	0x00000000, 0x00000000, 0x01000000, 0x01020408,
	0x08040201, 0x000f1133, 0x00000000, 0x00001f04,
	0x00001f04, 0x00001f04, 0x00001f04, 0x00001f04,
	0x00001f04, 0x00001f04, 0x00001f04, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00010000, 0x00020304,
	0x00000004, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x01010000,
	0x01000000, 0x03030000, 0x00010303, 0x01020202,
	0x00000000, 0x02040303, 0x21002103, 0x00061200,
	0x06120612, 0x04320432, 0x04320432, 0x00040004,
	0x00040004, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00010001
#else
#error Unsupported memory initialization
#endif
};

void __mxs_adjust_memory_params(uint32_t *dram_vals)
{
}
void mxs_adjust_memory_params(uint32_t *dram_vals)
	__attribute__((weak, alias("__mxs_adjust_memory_params")));

static void initialize_dram_values(void)
{
	int i;

	mxs_adjust_memory_params(dram_vals);

	for (i = 0; i < ARRAY_SIZE(dram_vals); i++)
		writel(dram_vals[i], MXS_DRAM_BASE + (4 * i));
}

static void mxs_mem_init_clock(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	/* Gate EMI clock */
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_set[CLKCTRL_FRAC0_EMI]);

	/* Set fractional divider for ref_emi to 480 * 18 / 21 = 411MHz */
	writeb(CLKCTRL_FRAC_CLKGATE | (21 & CLKCTRL_FRAC_FRAC_MASK),
		&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_EMI]);

	/* Ungate EMI clock */
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_clr[CLKCTRL_FRAC0_EMI]);

	early_delay(11000);

	/* Set EMI clock divider for EMI clock to 411 / 2 = 205MHz */
	writel((2 << CLKCTRL_EMI_DIV_EMI_OFFSET) |
		(1 << CLKCTRL_EMI_DIV_XTAL_OFFSET),
		&clkctrl_regs->hw_clkctrl_emi);

	/* Unbypass EMI */
	writel(CLKCTRL_CLKSEQ_BYPASS_EMI,
		&clkctrl_regs->hw_clkctrl_clkseq_clr);

	early_delay(10000);
}

static void mxs_mem_setup_cpu_and_hbus(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	/* Set fractional divider for ref_cpu to 480 * 18 / 19 = 454MHz
	 * and ungate CPU clock */
	writeb(19 & CLKCTRL_FRAC_FRAC_MASK,
		(uint8_t *)&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_CPU]);

	/* Set CPU bypass */
	writel(CLKCTRL_CLKSEQ_BYPASS_CPU,
		&clkctrl_regs->hw_clkctrl_clkseq_set);

	/* HBUS = 151MHz */
	writel(CLKCTRL_HBUS_DIV_MASK, &clkctrl_regs->hw_clkctrl_hbus_set);
	writel(((~3) << CLKCTRL_HBUS_DIV_OFFSET) & CLKCTRL_HBUS_DIV_MASK,
		&clkctrl_regs->hw_clkctrl_hbus_clr);

	early_delay(10000);

	/* CPU clock divider = 1 */
	clrsetbits_le32(&clkctrl_regs->hw_clkctrl_cpu,
			CLKCTRL_CPU_DIV_CPU_MASK, 1);

	/* Disable CPU bypass */
	writel(CLKCTRL_CLKSEQ_BYPASS_CPU,
		&clkctrl_regs->hw_clkctrl_clkseq_clr);

	early_delay(15000);
}

static void mxs_mem_setup_vdda(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	writel((0xc << POWER_VDDACTRL_TRG_OFFSET) |
		(0x7 << POWER_VDDACTRL_BO_OFFSET_OFFSET) |
		POWER_VDDACTRL_LINREG_OFFSET_1STEPS_BELOW,
		&power_regs->hw_power_vddactrl);
}

uint32_t mxs_mem_get_size(void)
{
	uint32_t sz, da;
	uint32_t *vt = (uint32_t *)0x20;
	/* The following is "subs pc, r14, #4", used as return from DABT. */
	const uint32_t data_abort_memdetect_handler = 0xe25ef004;

	/* Replace the DABT handler. */
	da = vt[4];
	vt[4] = data_abort_memdetect_handler;

	sz = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);

	/* Restore the old DABT handler. */
	vt[4] = da;

	return sz;
}

void mxs_mem_init(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
	struct mxs_pinctrl_regs *pinctrl_regs =
		(struct mxs_pinctrl_regs *)MXS_PINCTRL_BASE;

	/* Set DDR2 mode */
	writel(PINCTRL_EMI_DS_CTRL_DDR_MODE_DDR2,
		&pinctrl_regs->hw_pinctrl_emi_ds_ctrl_set);

	/* Power up PLL0 */
	writel(CLKCTRL_PLL0CTRL0_POWER,
		&clkctrl_regs->hw_clkctrl_pll0ctrl0_set);

	early_delay(11000);

	mxs_mem_init_clock();

	mxs_mem_setup_vdda();

	/*
	 * Configure the DRAM registers
	 */

	/* Clear START bit from DRAM_CTL16 */
	clrbits_le32(MXS_DRAM_BASE + 0x40, 1);

	initialize_dram_values();

	/* Clear SREFRESH bit from DRAM_CTL17 */
	clrbits_le32(MXS_DRAM_BASE + 0x44, 1);

	/* Set START bit in DRAM_CTL16 */
	setbits_le32(MXS_DRAM_BASE + 0x40, 1);

	/* Wait for bit 20 (DRAM init complete) in DRAM_CTL58 */
	while (!(readl(MXS_DRAM_BASE + 0xe8) & (1 << 20)))
		;

	early_delay(10000);

	mxs_mem_setup_cpu_and_hbus();
}
