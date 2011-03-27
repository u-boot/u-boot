/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
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
#include <ns16550.h>
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/sys_proto.h>

#include <asm/arch/clk_rst.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/uart.h>

DECLARE_GLOBAL_DATA_PTR;

const struct tegra2_sysinfo sysinfo = {
	CONFIG_TEGRA2_BOARD_STRING
};

/*
 * Routine: timer_init
 * Description: init the timestamp and lastinc value
 */
int timer_init(void)
{
	reset_timer();
	return 0;
}

/*
 * Routine: clock_init_uart
 * Description: init the PLL and clock for the UART(s)
 */
static void clock_init_uart(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	static int pllp_init_done;
	u32 reg;

	if (!pllp_init_done) {
		/* Override pllp setup for 216MHz operation. */
		reg = (PLL_BYPASS | PLL_BASE_OVRRIDE | PLL_DIVP);
		reg |= (((NVRM_PLLP_FIXED_FREQ_KHZ/500) << 8) | PLL_DIVM);
		writel(reg, &clkrst->crc_pllp_base);

		reg |= PLL_ENABLE;
		writel(reg, &clkrst->crc_pllp_base);

		reg &= ~PLL_BYPASS;
		writel(reg, &clkrst->crc_pllp_base);

		pllp_init_done++;
	}

	/* Now do the UART reset/clock enable */
#if defined(CONFIG_TEGRA2_ENABLE_UARTA)
	/* Assert Reset to UART */
	reg = readl(&clkrst->crc_rst_dev_l);
	reg |= SWR_UARTA_RST;		/* SWR_UARTA_RST = 1 */
	writel(reg, &clkrst->crc_rst_dev_l);

	/* Enable clk to UART */
	reg = readl(&clkrst->crc_clk_out_enb_l);
	reg |= CLK_ENB_UARTA;		/* CLK_ENB_UARTA = 1 */
	writel(reg, &clkrst->crc_clk_out_enb_l);

	/* Enable pllp_out0 to UART */
	reg = readl(&clkrst->crc_clk_src_uarta);
	reg &= 0x3FFFFFFF;	/* UARTA_CLK_SRC = 00, PLLP_OUT0 */
	writel(reg, &clkrst->crc_clk_src_uarta);

	/* wait for 2us */
	udelay(2);

	/* De-assert reset to UART */
	reg = readl(&clkrst->crc_rst_dev_l);
	reg &= ~SWR_UARTA_RST;		/* SWR_UARTA_RST = 0 */
	writel(reg, &clkrst->crc_rst_dev_l);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTA */
#if defined(CONFIG_TEGRA2_ENABLE_UARTD)
	/* Assert Reset to UART */
	reg = readl(&clkrst->crc_rst_dev_u);
	reg |= SWR_UARTD_RST;		/* SWR_UARTD_RST = 1 */
	writel(reg, &clkrst->crc_rst_dev_u);

	/* Enable clk to UART */
	reg = readl(&clkrst->crc_clk_out_enb_u);
	reg |= CLK_ENB_UARTD;		/* CLK_ENB_UARTD = 1 */
	writel(reg, &clkrst->crc_clk_out_enb_u);

	/* Enable pllp_out0 to UART */
	reg = readl(&clkrst->crc_clk_src_uartd);
	reg &= 0x3FFFFFFF;	/* UARTD_CLK_SRC = 00, PLLP_OUT0 */
	writel(reg, &clkrst->crc_clk_src_uartd);

	/* wait for 2us */
	udelay(2);

	/* De-assert reset to UART */
	reg = readl(&clkrst->crc_rst_dev_u);
	reg &= ~SWR_UARTD_RST;		/* SWR_UARTD_RST = 0 */
	writel(reg, &clkrst->crc_rst_dev_u);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTD */
}

/*
 * Routine: pin_mux_uart
 * Description: setup the pin muxes/tristate values for the UART(s)
 */
static void pin_mux_uart(void)
{
	struct pmux_tri_ctlr *pmt = (struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 reg;

#if defined(CONFIG_TEGRA2_ENABLE_UARTA)
	reg = readl(&pmt->pmt_ctl_c);
	reg &= 0xFFF0FFFF;	/* IRRX_/IRTX_SEL [19:16] = 00 UARTA */
	writel(reg, &pmt->pmt_ctl_c);

	reg = readl(&pmt->pmt_tri_a);
	reg &= ~Z_IRRX;		/* Z_IRRX = normal (0) */
	reg &= ~Z_IRTX;		/* Z_IRTX = normal (0) */
	writel(reg, &pmt->pmt_tri_a);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTA */
#if defined(CONFIG_TEGRA2_ENABLE_UARTD)
	reg = readl(&pmt->pmt_ctl_b);
	reg &= 0xFFFFFFF3;	/* GMC_SEL [3:2] = 00, UARTD */
	writel(reg, &pmt->pmt_ctl_b);

	reg = readl(&pmt->pmt_tri_a);
	reg &= ~Z_GMC;		/* Z_GMC = normal (0) */
	writel(reg, &pmt->pmt_tri_a);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTD */
}

/*
 * Routine: clock_init
 * Description: Do individual peripheral clock reset/enables
 */
void clock_init(void)
{
	clock_init_uart();
}

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pin_mux_uart();
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	/* boot param addr */
	gd->bd->bi_boot_params = (NV_PA_SDRAM_BASE + 0x100);
	/* board id for Linux */
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;

	/* Initialize peripheral clocks */
	clock_init();

	/* Initialize periph pinmuxes */
	pinmux_init();

	return 0;
}
