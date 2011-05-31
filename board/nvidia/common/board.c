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
#include "board.h"

#ifdef CONFIG_TEGRA2_MMC
#include <mmc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

const struct tegra2_sysinfo sysinfo = {
	CONFIG_TEGRA2_BOARD_STRING
};

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	/* Initialize periph clocks */
	clock_init();

	/* Initialize periph pinmuxes */
	pinmux_init();

	/* Initialize periph GPIOs */
	gpio_init();

	/* Init UART, scratch regs, and start CPU */
	tegra2_start();
	return 0;
}
#endif	/* EARLY_INIT */

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
	u32 reg;

	reg = readl(&clkrst->crc_pllp_base);
	if (!(reg & PLL_BASE_OVRRIDE)) {
		/* Override pllp setup for 216MHz operation. */
		reg = (PLL_BYPASS | PLL_BASE_OVRRIDE | PLL_DIVP);
		reg |= (((NVRM_PLLP_FIXED_FREQ_KHZ/500) << 8) | PLL_DIVM);
		writel(reg, &clkrst->crc_pllp_base);

		reg |= PLL_ENABLE;
		writel(reg, &clkrst->crc_pllp_base);

		reg &= ~PLL_BYPASS;
		writel(reg, &clkrst->crc_pllp_base);
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
 * Routine: clock_init_mmc
 * Description: init the PLL and clocks for the SDMMC controllers
 */
static void clock_init_mmc(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	/* Do the SDMMC resets/clock enables */

	/* Assert Reset to SDMMC4 */
	reg = readl(&clkrst->crc_rst_dev_l);
	reg |= SWR_SDMMC4_RST;		/* SWR_SDMMC4_RST = 1 */
	writel(reg, &clkrst->crc_rst_dev_l);

	/* Enable clk to SDMMC4 */
	reg = readl(&clkrst->crc_clk_out_enb_l);
	reg |= CLK_ENB_SDMMC4;		/* CLK_ENB_SDMMC4 = 1 */
	writel(reg, &clkrst->crc_clk_out_enb_l);

	/* Enable pllp_out0 to SDMMC4 */
	reg = readl(&clkrst->crc_clk_src_sdmmc4);
	reg &= 0x3FFFFF00;	/* SDMMC4_CLK_SRC = 00, PLLP_OUT0 */
	reg |= (10 << 1);	/* n-1, 11-1 shl 1 */
	writel(reg, &clkrst->crc_clk_src_sdmmc4);

	/*
	 * As per the Tegra2 TRM, section 5.3.4:
	 * 'Wait 2 us for the clock to flush through the pipe/logic'
	 */
	udelay(2);

	/* De-assert reset to SDMMC4 */
	reg = readl(&clkrst->crc_rst_dev_l);
	reg &= ~SWR_SDMMC4_RST;		/* SWR_SDMMC4_RST = 0 */
	writel(reg, &clkrst->crc_rst_dev_l);

	/* Assert Reset to SDMMC3 */
	reg = readl(&clkrst->crc_rst_dev_u);
	reg |= SWR_SDMMC3_RST;		/* SWR_SDMMC3_RST = 1 */
	writel(reg, &clkrst->crc_rst_dev_u);

	/* Enable clk to SDMMC3 */
	reg = readl(&clkrst->crc_clk_out_enb_u);
	reg |= CLK_ENB_SDMMC3;		/* CLK_ENB_SDMMC3 = 1 */
	writel(reg, &clkrst->crc_clk_out_enb_u);

	/* Enable pllp_out0 to SDMMC4, set divisor to 11 for 20MHz */
	reg = readl(&clkrst->crc_clk_src_sdmmc3);
	reg &= 0x3FFFFF00;	/* SDMMC3_CLK_SRC = 00, PLLP_OUT0 */
	reg |= (10 << 1);	/* n-1, 11-1 shl 1 */
	writel(reg, &clkrst->crc_clk_src_sdmmc3);

	/* wait for 2us */
	udelay(2);

	/* De-assert reset to SDMMC3 */
	reg = readl(&clkrst->crc_rst_dev_u);
	reg &= ~SWR_SDMMC3_RST;		/* SWR_SDMMC3_RST = 0 */
	writel(reg, &clkrst->crc_rst_dev_u);
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
static void pin_mux_mmc(void)
{
	struct pmux_tri_ctlr *pmt = (struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 reg;

	/* SDMMC4 */
	/* config 2, x8 on 2nd set of pins */
	reg = readl(&pmt->pmt_ctl_a);
	reg |= (3 << 16);	/* ATB_SEL [17:16] = 11 SDIO4 */
	writel(reg, &pmt->pmt_ctl_a);
	reg = readl(&pmt->pmt_ctl_b);
	reg |= (3 << 0);	/* GMA_SEL [1:0] = 11 SDIO4 */
	writel(reg, &pmt->pmt_ctl_b);
	reg = readl(&pmt->pmt_ctl_d);
	reg |= (3 << 0);	/* GME_SEL [1:0] = 11 SDIO4 */
	writel(reg, &pmt->pmt_ctl_d);

	reg = readl(&pmt->pmt_tri_a);
	reg &= ~Z_ATB;		/* Z_ATB = normal (0) */
	reg &= ~Z_GMA;		/* Z_GMA = normal (0) */
	writel(reg, &pmt->pmt_tri_a);
	reg = readl(&pmt->pmt_tri_b);
	reg &= ~Z_GME;		/* Z_GME = normal (0) */
	writel(reg, &pmt->pmt_tri_b);

	/* SDMMC3 */
	/* SDIO3_CLK, SDIO3_CMD, SDIO3_DAT[3:0] */
	reg = readl(&pmt->pmt_ctl_d);
	reg &= 0xFFFF03FF;
	reg |= (2 << 10);	/* SDB_SEL [11:10] = 01 SDIO3 */
	reg |= (2 << 12);	/* SDC_SEL [13:12] = 01 SDIO3 */
	reg |= (2 << 14);	/* SDD_SEL [15:14] = 01 SDIO3 */
	writel(reg, &pmt->pmt_ctl_d);

	reg = readl(&pmt->pmt_tri_b);
	reg &= ~Z_SDC;		/* Z_SDC = normal (0) */
	reg &= ~Z_SDD;		/* Z_SDD = normal (0) */
	writel(reg, &pmt->pmt_tri_b);
	reg = readl(&pmt->pmt_tri_d);
	reg &= ~Z_SDB;		/* Z_SDB = normal (0) */
	writel(reg, &pmt->pmt_tri_d);
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
 * Routine: gpio_init
 * Description: Do individual peripheral GPIO configs
 */
void gpio_init(void)
{
	gpio_config_uart();
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

	return 0;
}

#ifdef CONFIG_TEGRA2_MMC
/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("board_mmc_init called\n");
	/* Enable clocks, muxes, etc. for SDMMC controllers */
	clock_init_mmc();
	pin_mux_mmc();

	debug("board_mmc_init: init eMMC\n");
	/* init dev 0, eMMC chip, with 4-bit bus */
	tegra2_mmc_init(0, 4);

	debug("board_mmc_init: init SD slot\n");
	/* init dev 1, SD slot, with 4-bit bus */
	tegra2_mmc_init(1, 4);

	return 0;
}

/* this is a weak define that we are overriding */
int board_mmc_getcd(u8 *cd, struct mmc *mmc)
{
	debug("board_mmc_getcd called\n");
	/*
	 * Hard-code CD presence for now. Need to add GPIO inputs
	 * for Seaboard & Harmony (& Kaen/Aebl/Wario?)
	 */
	*cd = 1;
	return 0;
}
#endif
